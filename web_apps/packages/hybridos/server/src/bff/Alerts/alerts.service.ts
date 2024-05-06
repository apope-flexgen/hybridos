import { Inject, Injectable } from '@nestjs/common'
import { ActiveAlertsResponse, ResolveAlertResponse, ResolvedAlertsResponse } from './responses/alerts.response'
import { AlertConfiguration, AlertConfigurationsResponse, Alias, Comparator, Deadline, Duration, Expression, Template } from './responses/alertConfig.response'
import { AlertsRequest } from './dtos/alerts.dto'
import {
    FimsMsg,
    FIMS_SERVICE,
    IFimsService,
} from '../../fims/interfaces/fims.interface';
import { Observable, map } from 'rxjs'
import { AlertConfigDTO } from './dtos/alertConfig.dto'
import { AlertsPostResponse } from './responses/alertPost.response'
import { AlertURIs, AlertConditionalsMapping, LogicalOperators } from './alerts.constants';
import { OrganizationsDTO } from './dtos/organizations.dto';
import { Organization, OrganizationsResponse } from './responses/organizations.response'

@Injectable()
export class AlertsService {
    constructor(@Inject(FIMS_SERVICE) private readonly fimsService: IFimsService) {}
    private parseQueryToFilters (query: AlertsRequest) {
        const { resolvedFilter, severityFilter, limit, page } = query;
        const filter = {
            ...query,
            resolvedFilter: resolvedFilter ? `${resolvedFilter}`.toLowerCase() === 'true' : null,
            severityFilter: severityFilter !== null && severityFilter !== undefined ? parseInt(severityFilter.toString()) : null,
            limit: limit ? parseInt(limit) : null,
            page: page ? parseInt(page.toString()) : null,
        }

        Object.keys(filter).forEach(key => filter[key] === undefined || filter[key] === null ? delete filter[key] : {});

        return filter;
    }

    async activeAlerts(query: AlertsRequest): Promise<ActiveAlertsResponse> {
        const filters = this.parseQueryToFilters(query);
        const fimsResponse: FimsMsg = await this.fimsService.get(AlertURIs.ALERT_INSTANCES, filters)
        const { rows, count } = fimsResponse.body;
        return { data: rows, count }
    }

    private convertToSeconds(value: number | string, unit: 'minute' | 'hour' | 'second'){
        if (unit === 'minute') return Number(value) * 60;
        if (unit === 'hour') return Number(value) * 3600;
        return Number(value);
    }

    private parseAliasForGoMetrics(alias: string): string {
        return alias.toString().replace(/ /g,"_")
    }

    private parseAliasForUI(alias: string): string {
        const fragments = alias.split('_');
        return fragments.join(' ');
    }

    private convertDurationToProperUnit(value: string | number): Duration {
        if (Number(value) % 3600  === 0) return {value: Number(value)/3600, unit: 'hour'}
        if (Number(value) % 60 === 0) return {value: Number(value)/60, unit: 'minute'}
        return {value, unit: 'second'}
    };

    private parseExpressionToConditions(expression: string, aliases: Alias[], messages: string[]): Expression[] {
        if (expression) {
            const andOrRegEx = /[\||\&&]+/g;
            const listOfConditions = expression.split(andOrRegEx);
            const listOfConnectionOperators: ("or" | "and")[] = [...expression.matchAll(andOrRegEx)].map((match) => match[0] === '||' ? 'or' : 'and');
        
            const conditions: Expression[] = listOfConditions.map((condition, index) => {
                let expressionWithoutDuration = condition
                let durationSeconds = ''
                if (condition.includes('Duration')) {
                    const expressionWithoutParens = condition.split(/[\)\(]+/g)[1];
                    durationSeconds = expressionWithoutParens.split(',')[1];
                    const restOfExpression = expressionWithoutParens.split(',')[0];
                    expressionWithoutDuration = restOfExpression
                }
    
                let conditional = '';
                LogicalOperators.forEach((operator) => {
                    if (expressionWithoutDuration.includes(` ${operator} `)) conditional = operator
                })
    
               const comparator1Value = expressionWithoutDuration.split(conditional)[0].trim()
    
               const comparator1: Comparator = {
                type: 'alias',
                value: this.parseAliasForUI(comparator1Value)
            }
    
                const comparator2Value = expressionWithoutDuration.split(conditional)[1].trim();
                const comparator2Type = aliases.some((alias) => comparator2Value.includes(this.parseAliasForGoMetrics(alias.alias))) ? 'alias' : 'literal';
                const comparator2: Comparator = {
                    type: comparator2Type,
                    value: comparator2Type === 'alias' ?  this.parseAliasForUI(comparator2Value) : comparator2Value,
                }
    
                let connectionOperator = index !== 0 ? listOfConnectionOperators[index - 1] : null;
                
                const message = messages.find((message) => message[condition.trim()] !== undefined);

                const mappedCondition: Expression = {
                    index,
                    connectionOperator,
                    comparator1,
                    conditional,
                    comparator2, 
                    duration: durationSeconds ? this.convertDurationToProperUnit(durationSeconds) : undefined,
                    message: message ? message[condition.trim()] : ''
                }
    
                return mappedCondition;
            })
            return conditions
        }
        return []
    }

    private parseConditionsToExpression(conditions: Expression[], aliases: Alias[]): {expression: string, messages: {[key: string]: string}[]} {
        const sortedExpressions = conditions.sort((expression1, expression2) => {return expression1.index - expression2.index});

        let expressionString = '';
        let messages: { [key: string]: string }[] = [];

        sortedExpressions.forEach((expression) => {
            const comparator1Value = this.parseAliasForGoMetrics(expression.comparator1.value.toString());
            const comparator2Value = expression.comparator2.type === 'alias' ? this.parseAliasForGoMetrics(expression.comparator2.value.toString()) : expression.comparator2.value;
            const connectionOperator = expression.connectionOperator ? AlertConditionalsMapping[expression.connectionOperator] : '';

            let newExpression = 
                `${comparator1Value} ${expression.conditional} ${comparator2Value}`
            if (expression.duration) {
                const durationInSeconds = this.convertToSeconds(expression.duration.value, expression.duration.unit)
                newExpression = `Duration(${newExpression}, ${durationInSeconds})`
            }

            // if the user formatted values for aliases to be displayed in the message
            // parse the aliases correctly so go metrics can understand them
            // proper formatting -> {ESS 2 SOC} -> {ess_2_soc}
            const bracesRegex =  /\{(.*?)\}/g;
            let formattedMessage = '';
             expression.message.split(bracesRegex).forEach(
                (piece) => {
                    const alias = aliases.find((alias: Alias) =>  `${alias.alias}` === piece)
                    if (alias) formattedMessage += `{${this.parseAliasForGoMetrics(alias.alias)}}`
                    else formattedMessage += `${piece}`
                }
            )

            messages.push({ [newExpression]: formattedMessage })

            newExpression = ` ${connectionOperator} ${newExpression}`

            expressionString += newExpression
        })

        return {expression: expressionString.trim(), messages};
    }

    private convertDeadlineToProperUnit(value: string | number): Deadline {
        if (Number(value) % 60 === 0) return {value: Number(value)/60, unit: 'hour'}
        return {value, unit: 'minute'}
    };

    private parseAlertConfigurationsFromEvents(rawData: any[]): AlertConfiguration[] {
        const alertConfigurations = rawData
            .filter((alert) => !alert.deleted)
            .map((alert) => ({
                id: alert.id,
                severity: alert.severity,
                organization: alert.organization,
                deadline: this.convertDeadlineToProperUnit(alert.deadline),
                conditions: this.parseExpressionToConditions(alert.expression, alert.aliases, alert.messages),
                title: alert.title || '',
                enabled: alert.enabled,
                last_trigger_time: alert.lastTriggered || '',
                aliases: alert.aliases?.length > 0 
                    ? alert.aliases.map((aliasObject, index) => ({
                        ...aliasObject, 
                        id: index, 
                        alias: this.parseAliasForUI(aliasObject.alias),
                        type: aliasObject.type === 'float' || aliasObject.type === 'int' ? 'number' : aliasObject.type
                    })) 
                    : alert.aliases,
            }))
        return alertConfigurations;
    }

    
    private convertToMinutes(value: string | number, unit: 'minute' | 'hour'): number {
        if (unit === 'minute') return Number(value);
        return (Number(value) * 60);
    };

    async addNewConfig(alertConfigDTO: AlertConfigDTO, username: string): Promise<AlertsPostResponse> {
        const { title, deadline, severity, organization, aliases, templates, conditions, enabled } = alertConfigDTO
        const aliasesWithProperType = aliases.map((alias) => ({...alias, type: alias.type === 'number' ? 'float' : alias.type}))
        const aliasesWithProperFormatting = aliasesWithProperType.map((aliasObject) => ({...aliasObject, alias: this.parseAliasForGoMetrics(aliasObject.alias)}))
        const deadlineInMinutes = this.convertToMinutes(deadline.value, deadline.unit);
        const { expression, messages } = this.parseConditionsToExpression(conditions, aliases);
        const newAlertConfig = {
            title, deadline: deadlineInMinutes, messages, sites: [], severity, organization, templates, expression, aliases: aliasesWithProperFormatting, enabled
        }

        const postNewAlertResponse = await this.fimsService.send({
            method: 'post',
            uri:  AlertURIs.ALERT_CONFIGS,
            replyto: '/web_server/alerts/add_new_config',
            body: JSON.stringify(newAlertConfig),
            username: username,
        })
        return postNewAlertResponse.body as AlertsPostResponse;
    }
    
    async updateConfig(id: string, alertConfigDTO: AlertConfigDTO, username: string): Promise<AlertsPostResponse> {
        const { title, deadline, severity, organization, aliases, templates, conditions, enabled, deleted } = alertConfigDTO

        const aliasesWithProperType = aliases ? aliases.map((alias) => ({...alias, type: alias.type === 'number' ? 'float' : alias.type})) : null
        const deadlineInMinutes = deadline ? this.convertToMinutes(deadline.value, deadline.unit) : null;
        const {expression, messages} = conditions ? this.parseConditionsToExpression(conditions, aliases) : null;

        const newAlertConfig = {
            title, deadline: deadlineInMinutes, messages, severity, organization, templates, expression, aliases: aliasesWithProperType, enabled, deleted
        }

        Object.keys(newAlertConfig).forEach(key => newAlertConfig[key] === undefined || newAlertConfig[key] === null ? delete newAlertConfig[key] : {});

        const updatedConfig = {
            ...newAlertConfig,
            id,
        }

        const postNewAlertResponse = await this.fimsService.send({
            method: 'post',
            uri: AlertURIs.ALERT_CONFIGS,
            replyto: '/web_server/alerts/update_config',
            body: JSON.stringify(updatedConfig),
            username: username,
        })

        return postNewAlertResponse.body as AlertsPostResponse;
    }

    async alertConfigurations(): Promise<AlertConfigurationsResponse> {
        const fimsResponse: FimsMsg = await this.fimsService.get(AlertURIs.ALERT_CONFIGS)
        const data = this.parseAlertConfigurationsFromEvents(fimsResponse.body.rows || [])
        const templates: Template[] = fimsResponse.body.templates

        return { data, templates }
    }

    getAlertingObservable = (): Observable<any> => {
        const fimsSubscribe = this.fimsService.subscribe(AlertURIs.ALERT_INSTANCES)

        const newObservable: Observable<any> = fimsSubscribe.pipe(
            map((event) => {
                return { data: event.body }
            })
        )

        return newObservable
    }

    async resolveAlert(
        id: string,
        message: string,
        username: string
    ): Promise<ResolveAlertResponse> {
        const fimsResponse = await this.fimsService.send({
            method: 'set',
            uri: `${AlertURIs.ALERT_INSTANCES}/${id}`,
            replyto: `/web_server/alerts/${id}`,
            body: JSON.stringify({
                resolved: true,
                resolution_message: message
            }),
            username: username,
        });
        
        return fimsResponse.body as ResolveAlertResponse;
    }

    async resolvedAlerts(query: AlertsRequest): Promise<ResolvedAlertsResponse> {
        const filters = this.parseQueryToFilters(query);
        const dataFromFims: FimsMsg = await this.fimsService.get(AlertURIs.ALERT_INSTANCES, filters);
        const { rows, count } = dataFromFims.body;
        return { data: rows, count };
    }

    async editOrganizations(newOrganizations: OrganizationsDTO, username: string): Promise<ResolveAlertResponse> {
        const editOrganizationsResponse = await this.fimsService.send({
            method: 'post',
            uri: `${AlertURIs.ALERT_INSTANCES}/organizations`,
            replyto: '/web_server/alerts/edit_organizations',
            body: JSON.stringify({ rows: newOrganizations.organizations }),
            username: username,
        })

        return editOrganizationsResponse.body as ResolveAlertResponse;
    }

    async getOrganizations(): Promise<OrganizationsResponse> {
        const getOrganizationsResponse = await this.fimsService.get(`${AlertURIs.ALERT_INSTANCES}/organizations`)
        const organizations = getOrganizationsResponse.body as Organization[];

        return { data: organizations };
    }

    async deleteOrganization(id: string, username: string): Promise<ResolveAlertResponse> {
        const deleteOrganizationResponse = await this.fimsService.send({
            method: 'del',
            uri: `${AlertURIs.ALERT_INSTANCES}/organizations`,
            replyto: '/web_server/alerts/delete_organizations',
            body: { id },
            username: username,
        })
        return deleteOrganizationResponse.body as ResolveAlertResponse;
    }
}
