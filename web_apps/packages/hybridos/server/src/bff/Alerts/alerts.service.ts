import { Inject, Injectable } from '@nestjs/common';
import {
  ActiveAlertsResponse,
  ResolveAlertResponse,
  ResolvedAlertsResponse,
} from './responses/alerts.response';
import {
  AlertConfiguration,
  AlertConfigurationsResponse,
  Deadline,
  Template,
} from './responses/alertConfig.response';
import { AlertsRequest } from './dtos/alerts.dto';
import { FimsMsg, FIMS_SERVICE, IFimsService } from '../../fims/interfaces/fims.interface';
import { Observable, map } from 'rxjs';
import { AlertConfigDTO } from './dtos/alertConfig.dto';
import { AlertsPostResponse } from './responses/alertPost.response';
import { AlertURIs } from './alerts.constants';
import { OrganizationsDTO } from './dtos/organizations.dto';
import { Organization, OrganizationsResponse } from './responses/organizations.response';
import {
  parseAliasForUI,
  parseConditionsToExpression,
  parseExpressionToConditions,
  parseAliasForGoMetrics,
} from './alerts.parser';

@Injectable()
export class AlertsService {
  constructor(@Inject(FIMS_SERVICE) private readonly fimsService: IFimsService) {}

  private parseQueryToFilters(query: AlertsRequest) {
    const { resolvedFilter, severityFilter, limit, page } = query;

    const filter = {
      ...query,
      resolvedFilter: resolvedFilter ? `${resolvedFilter}`.toLowerCase() === 'true' : null,
      severityFilter:
        severityFilter !== null && severityFilter !== undefined
          ? parseInt(severityFilter.toString())
          : null,
      limit: limit ? parseInt(limit) : null,
      page: page ? parseInt(page.toString()) : null,
    };

    Object.keys(filter).forEach((key) =>
      filter[key] === undefined || filter[key] === null ? delete filter[key] : {},
    );

    return filter;
  }

  async activeAlerts(query: AlertsRequest): Promise<ActiveAlertsResponse> {
    const filters = this.parseQueryToFilters(query);
    const fimsResponse: FimsMsg = await this.fimsService.get(AlertURIs.ALERT_INSTANCES, filters);
    const { rows, count } = fimsResponse.body;
    return { data: rows, count };
  }

  private convertDeadlineToProperUnit(value: string | number): Deadline {
    if (Number(value) % 60 === 0) return { value: Number(value) / 60, unit: 'hour' };
    return { value, unit: 'minute' };
  }

  private parseAlertConfigurationsFromEvents(rawData: any[]): AlertConfiguration[] {
    const alertConfigurations = rawData
      .filter((alert) => !alert.deleted)
      .map((alert) => ({
        id: alert.id,
        severity: alert.severity,
        organization: alert.organization,
        deadline: this.convertDeadlineToProperUnit(alert.deadline),
        conditions: parseExpressionToConditions(alert.expression, alert.aliases, alert.messages),
        title: alert.title || '',
        enabled: alert.enabled,
        last_trigger_time: alert.lastTriggered || '',
        aliases:
          alert.aliases?.length > 0
            ? alert.aliases.map((aliasObject, index) => ({
                ...aliasObject,
                id: index,
                alias: parseAliasForUI(aliasObject.alias),
                type:
                  aliasObject.type === 'float' || aliasObject.type === 'int'
                    ? 'number'
                    : aliasObject.type,
                uri:
                  aliasObject.uri.charAt(0) === '/'
                    ? aliasObject.uri.substring(1)
                    : aliasObject.uri,
              }))
            : alert.aliases,
      }));
    return alertConfigurations;
  }

  private convertToMinutes(value: string | number, unit: 'minute' | 'hour'): number {
    if (unit === 'minute') return Number(value);
    return Number(value) * 60;
  }

  private parseAlertConfigForPost(alertConfigDTO: AlertConfigDTO) {
    const aliasesWithProperType = alertConfigDTO.aliases
      ? alertConfigDTO.aliases.map((alias) => ({
          ...alias,
          type: alias.type === 'number' ? 'float' : alias.type,
        }))
      : null;
    const aliasesWithProperFormatting = aliasesWithProperType.map((aliasObject) => ({
      ...aliasObject,
      alias: parseAliasForGoMetrics(aliasObject.alias),
      uri: aliasObject.uri.charAt(0) === '/' ? aliasObject.uri : `/${aliasObject.uri}`,
    }));
    const deadlineInMinutes = alertConfigDTO.deadline
      ? this.convertToMinutes(alertConfigDTO.deadline.value, alertConfigDTO.deadline.unit)
      : null;
    const { expression, messages } = alertConfigDTO.conditions
      ? parseConditionsToExpression(
          alertConfigDTO.conditions,
          alertConfigDTO.aliases,
          alertConfigDTO.templates || [],
        )
      : null;

    const newAlertConfig = {
      title: alertConfigDTO.title,
      deadline: deadlineInMinutes,
      messages,
      severity: alertConfigDTO.severity,
      organization: alertConfigDTO.organization,
      templates: (alertConfigDTO.templates || []).map((template) => {
        const clone = template;
        if (template.id && template.id.includes('temp')) delete clone.id;
        return {
          ...clone,
          token: `{${template.token}}`,
        };
      }),
      expression,
      aliases: aliasesWithProperFormatting,
      enabled: alertConfigDTO.enabled,
      deleted: alertConfigDTO.deleted,
    };

    Object.keys(newAlertConfig).forEach((key) =>
      newAlertConfig[key] === undefined || newAlertConfig[key] === null
        ? delete newAlertConfig[key]
        : {},
    );

    return newAlertConfig;
  }

  async addNewConfig(
    alertConfigDTO: AlertConfigDTO,
    username: string,
  ): Promise<AlertsPostResponse> {
    const newAlertConfig = this.parseAlertConfigForPost(alertConfigDTO);

    const postNewAlertResponse = await this.fimsService.send({
      method: 'post',
      uri: AlertURIs.ALERT_CONFIGS,
      replyto: '/web_server/alerts/add_new_config',
      body: JSON.stringify(newAlertConfig),
      username: username,
    });
    return postNewAlertResponse.body as AlertsPostResponse;
  }

  async updateConfig(
    id: string,
    alertConfigDTO: AlertConfigDTO,
    username: string,
  ): Promise<AlertsPostResponse> {
    const newAlertConfig = this.parseAlertConfigForPost(alertConfigDTO);

    const updatedConfig = {
      ...newAlertConfig,
      id,
    };

    const postNewAlertResponse = await this.fimsService.send({
      method: 'post',
      uri: AlertURIs.ALERT_CONFIGS,
      replyto: '/web_server/alerts/update_config',
      body: JSON.stringify(updatedConfig),
      username: username,
    });

    return postNewAlertResponse.body as AlertsPostResponse;
  }

  async alertConfigurations(): Promise<AlertConfigurationsResponse> {
    const fimsResponse: FimsMsg = await this.fimsService.get(AlertURIs.ALERT_CONFIGS);
    const data = this.parseAlertConfigurationsFromEvents(fimsResponse.body.rows || []);
    const templates: Template[] = fimsResponse.body.templates;
    const formattedTemplates = templates.map((template) => ({
      ...template,
      token: template.token.replaceAll('{', '').replaceAll('}', ''),
    }));

    return { data, templates: formattedTemplates };
  }

  getAlertingObservable = (): Observable<any> => {
    const fimsSubscribe = this.fimsService.subscribe(AlertURIs.ALERT_INSTANCES);

    const newObservable: Observable<any> = fimsSubscribe.pipe(
      map((event) => {
        return { data: event.body };
      }),
    );

    return newObservable;
  };

  async resolveAlert(id: string, message: string, username: string): Promise<ResolveAlertResponse> {
    const fimsResponse = await this.fimsService.send({
      method: 'set',
      uri: `${AlertURIs.ALERT_INSTANCES}/${id}`,
      replyto: `/web_server/alerts/${id}`,
      body: JSON.stringify({
        resolved: true,
        resolution_message: message,
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

  async editOrganizations(
    newOrganizations: OrganizationsDTO,
    username: string,
  ): Promise<ResolveAlertResponse> {
    const editOrganizationsResponse = await this.fimsService.send({
      method: 'post',
      uri: `${AlertURIs.ALERT_INSTANCES}/organizations`,
      replyto: '/web_server/alerts/edit_organizations',
      body: JSON.stringify({ rows: newOrganizations.organizations }),
      username: username,
    });

    return editOrganizationsResponse.body as ResolveAlertResponse;
  }

  async getOrganizations(): Promise<OrganizationsResponse> {
    const getOrganizationsResponse = await this.fimsService.get(
      `${AlertURIs.ALERT_INSTANCES}/organizations`,
    );
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
    });
    return deleteOrganizationResponse.body as ResolveAlertResponse;
  }
}
