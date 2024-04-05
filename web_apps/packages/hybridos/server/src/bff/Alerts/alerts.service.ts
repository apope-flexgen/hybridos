import { Inject, Injectable } from '@nestjs/common'
import { ActiveAlertsResponse, ResolveAlertResponse, ResolvedAlertsResponse } from './responses/alerts.response'
import { AlertConfiguration, AlertConfigurationsResponse, Expression } from './responses/alertConfig.response'
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

    private convertToSeconds(value: number | string, unit: 'minutes' | 'seconds' | 'hours'){
        if (unit === 'minutes') return Number(value) * 60;
        if (unit === 'hours') return Number(value) * 3600;
        return Number(value)
    }

    private parseConditionsToExpression(conditions: Expression[]): string {
        const sortedExpressions = conditions.sort((expresion1, expression2) => {return expresion1.index - expression2.index});

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


    private parseAlertConfigurationsFromEvents(rawData: any[]): AlertConfiguration[] {
        const alertConfigurations = rawData
            .filter((alert) => !alert.deleted)
            .map((alert) => ({
                id: alert.id,
                severity: alert.severity,
                organization: alert.organization,
                deadline: alert.deadline,
                conditions: this.parseExpressionToConditions(alert.expression),
                sites: alert.sites || [],
                title: alert.title || '',
                enabled: alert.enabled,
                last_trigger_time: alert.lastTriggered || '',
                aliases: alert.aliases || [],
                templates: alert.templates || [],
            }))
        return alertConfigurations;
    }

    async addNewConfig(alertConfigDTO: AlertConfigDTO, username: string): Promise<AlertsPostResponse> {
        const { title, deadline, severity, organization, sites, aliases, templates, conditions } = alertConfigDTO
        const expression = this.parseConditionsToExpression(conditions);
        const newAlertConfig = {
            title, sites, deadline, severity, organization, templates, expression, aliases
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
        Object.keys(alertConfigDTO).forEach(key => alertConfigDTO[key] === undefined || alertConfigDTO[key] === null ? delete alertConfigDTO[key] : {});

        const updatedConfig = {
            ...alertConfigDTO,
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
