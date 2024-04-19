import { Inject, Injectable } from '@nestjs/common'
import { ActiveAlertsResponse, ResolveAlertResponse, ResolvedAlertsResponse } from './responses/alerts.response'
import { AlertConfiguration, AlertConfigurationsResponse, Deadline, Expression } from './responses/alertConfig.response'
import { AlertsRequest } from './dtos/alerts.dto'
import {
    FimsMsg,
    FIMS_SERVICE,
    IFimsService,
} from '../../fims/interfaces/fims.interface';
import { Observable, map } from 'rxjs'
import { AlertConfigDTO } from './dtos/alertConfig.dto'
import { AlertsPostResponse } from './responses/alertPost.response'
import { AlertURIs } from './alerts.constants';

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

    private parseExpressionToConditions(expression: string): Expression[] {
        // TODO: add actual implementation to interpret expression as user friendly conditions
        return []
    }

    private convertToSeconds(value: number | string, unit: 'minute' | 'hour' | 'second'){
        if (unit === 'minute') return Number(value) * 60;
        if (unit === 'hour') return Number(value) * 3600;
        return Number(value);
    }

    private parseConditionsToExpression(conditions: Expression[]): string {
        const sortedExpressions = conditions.sort((expression1, expression2) => {return expression1.index - expression2.index});

        let expressionString = '';
        sortedExpressions.forEach((expression) => {
            let newExpression = 
                `${expression.connectionOperator ? expression.connectionOperator : ''} ${expression.operand1} ${expression.operator} ${expression.operand2} `
            if (expression.duration) {
                const durationInSeconds = this.convertToSeconds(expression.duration.value, expression.duration.unit)
                newExpression = `Duration(${newExpression}, ${durationInSeconds})`
            }
            expressionString += newExpression
        })
        
        return expressionString;
    }

    private convertToProperUnit(value: string | number): Deadline {
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
                deadline: this.convertToProperUnit(alert.deadline),
                conditions: this.parseExpressionToConditions(alert.expression),
                sites: alert.sites || [],
                title: alert.title || '',
                enabled: alert.enabled,
                last_trigger_time: alert.lastTriggered || '',
                aliases: alert.aliases?.length > 0 
                    ? alert.aliases.map((alias, index) => ({
                        ...alias, 
                        id: index, 
                        type: alias.type === 'float' || alias.type === 'int' ? 'number' : alias.type
                    })) 
                    : alert.aliases,
                templates: alert.templates?.length > 0 ? alert.templates.map((template, index) =>({...template, id: index})): alert.templates,
            }))
        return alertConfigurations;
    }

    
    private convertToMinutes(value: string | number, unit: 'minute' | 'hour'): number {
        if (unit === 'minute') return Number(value);
        return (Number(value) * 60);
    };

    async addNewConfig(alertConfigDTO: AlertConfigDTO, username: string): Promise<AlertsPostResponse> {
        const { title, deadline, severity, organization, sites, aliases, templates, conditions, enabled } = alertConfigDTO
        const aliasesWithProperType = aliases.map((alias) => ({...alias, type: alias.type === 'number' ? 'float' : alias.type}))
        const deadlineInMinutes = this.convertToMinutes(deadline.value, deadline.unit);
        const expression = this.parseConditionsToExpression(conditions);
        const newAlertConfig = {
            title, sites, deadline: deadlineInMinutes, severity, organization, templates, expression, aliases: aliasesWithProperType, enabled
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
        const { title, deadline, severity, organization, sites, aliases, templates, conditions, enabled, deleted } = alertConfigDTO

        const aliasesWithProperType = aliases ? aliases.map((alias) => ({...alias, type: alias.type === 'number' ? 'float' : alias.type})) : null
        const deadlineInMinutes = deadline ? this.convertToMinutes(deadline.value, deadline.unit) : null;
        const expression = conditions ? this.parseConditionsToExpression(conditions) : null;

        const newAlertConfig = {
            title, sites, deadline: deadlineInMinutes, severity, organization, templates, expression, aliases: aliasesWithProperType, enabled, deleted
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
        const data = this.parseAlertConfigurationsFromEvents(fimsResponse.body)

        return { data }
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
}
