/* eslint-disable max-lines */
/* eslint-disable max-len */
import { Column } from '@flexgen/storybook';

import {
  AlertConfigurationObject,
  Alias,
  Expression,
  Template,
} from 'src/pages/ActivityLog/activityLog.types';

export type AlertManagementPageLayout = 'table' | 'form';

export const AlertManagementPageLayouts: {
  [key: string]: AlertManagementPageLayout;
} = {
  TABLE: 'table',
  FORM: 'form',
};

export const checkForUniqueness = (alias: Alias, allAliases: Alias[], templates: Template[]) => {
  const aliasesWithoutCurrent = allAliases.filter((existingAlias) => alias.id !== existingAlias.id);
  const names = aliasesWithoutCurrent.map((allAlias) => allAlias.alias);
  const wildcards = templates?.map((template) => template.token) || [];
  return (
    names.some((name) => name === alias.alias)
    || wildcards.some((wildcard) => wildcard === alias.alias)
  );
};

export const insertAtCursor = (newValue: string, field: HTMLInputElement) => {
  const currentValue = field.value;
  if (field.selectionStart || field.selectionStart === 0) {
    const startPos = field.selectionStart || 0;
    const endPos = field.selectionEnd || 0;
    return (
      currentValue.substring(0, startPos)
      + newValue
      + currentValue.substring(endPos, currentValue.length)
    );
  }
  return currentValue + newValue;
};

const getExampleAliasValue = (alias: Alias) => {
  switch (alias.type) {
    case 'number':
      return '1.00';
    case 'boolean':
      return 'true';
    case 'string':
      return 'string_value';
    default:
      return '';
  }
};

const getExampleTemplateValue = (template: Template) => {
  switch (template.type) {
    case 'list':
      return template.list?.[0] || '';
    case 'sequential':
      return (template.from?.toString() || '').padStart(template.minWidth || 1, '0');
    case 'range':
      return template.range?.[0].split('..')[0].padStart(template.minWidth || 1, '0') || '';
    default:
      return '';
  }
};

export const generateExampleText = (alertValues: AlertConfigurationObject, message: string) => {
  let exampleMessage = 'Example Output: ';
  const bracesRegex = /\{(.*?)\}/g;
  message.split(bracesRegex).forEach((piece) => {
    const aliasInMessage = alertValues.aliases?.find((alias: Alias) => alias.alias && `${alias.alias}` === piece) || '';
    const templateInMessage = alertValues.templates?.find(
      (template: Template) => `${template.token}` && `${template.token}` === piece,
    ) || '';
    if (templateInMessage) exampleMessage += getExampleTemplateValue(templateInMessage);
    else if (aliasInMessage) exampleMessage += getExampleAliasValue(aliasInMessage);
    else exampleMessage += `${piece}`;
  });
  return exampleMessage;
};

export const alertManagementHelperText = {
  aliasWildcard:
    'To configure a templated URI, place a wildcard from the list above in braces to denote where the replacement should occur.',
  deletingOrganizations:
    'Deleting an organization that has existing alerts configured for it will cause the existing alerts to be deleted as well.',
  editOrganizations: 'Use this field to add, delete, or modify your organizations.',
  orgEmptyError: 'Organizations cannot be empty',
  orgDuplicateError: 'Organizations cannot have duplicate names',
  alertInfo: 'A title and severity for this alert',
  aliases: 'Aliases to use in your rule logic below',
  aliasURI: 'URIs should begin with the / character',
  requiredResponseTime: 'How quickly must this alert be resolved by personnel',
  ruleLogic: 'Settings that will trigger this alert',
  expressionMessageFormatting:
    'An alias placed in braces within the message will be replaced with the actual value incoming from that alias',
  expressionMessageFormattingTemplates:
    'A template placed in braces will be replaced with the replacement value specified within the template configuration',
  expressionMessageExample:
    'If your alias were SOC Value and your template wildcard ess_num, "ESS {ess_num} SOC reached {SOC Value}%" -> "ESS 1 SOC reached 90%"',
  scope: 'Which organization should receive this alert',
  templates: 'Any templated wildcards to utilize in the aliases below',
  templateReplacementValues:
    'Enter a list of comma separated values to replace the wildcard character defined.',
  templateRangeReplacementValues: 'Ranges can be denoted with the following format - xx..xx.',
  templateRangeExample: '1..10 will become 1 through 10',
  templateWildcard:
    'Chose a wildcard character that will be replaced with the replacement values configured below',
  templateWildcardExample: 'e.g. ess, site, site_id',
  padStart:
    'Substitution values will be prefixed with zeroes to maintain the specified minimum width.',
  padStartExample1: 'If minimum width is 2, 1..10 will become 01..10',
};

export const generateInitialAliases = (aliases: Alias[]): Alias[] => [
  {
    alias: '',
    uri: '',
    type: 'number',
    id: aliases ? `${aliases.length + 1}` : '0',
  },
];

export const generateInitialTemplates = (templates: Template[]): Template[] => [
  {
    token: '',
    list: [],
    type: 'sequential',
    id: `temp${(templates?.length || 0) + 1}`,
  },
];

export const initialAlertManagementFilters = {
  limit: 50,
  page: 0,
  order: -1,
  orderBy: 'lastUpdated',
};

export const severities = ['Critical', 'High', 'Medium', 'Low'];

export const initialNewExpression = (conditions: Expression[]): Expression => ({
  index: conditions?.length > 0 ? conditions.length + 1 : 0,
  connectionOperator: conditions?.length > 0 ? 'or' : null,
  comparator1: { value: '', type: 'alias' },
  conditional: '==',
  comparator2: { value: '', type: 'literal' },
  message: '',
});

export const convertToMinutes = (value: string, unit: 'Minutes' | 'Hours') => {
  if (unit === 'Minutes') return Number(value);
  return Number(value) * 60;
};

export const initialNewAlert: AlertConfigurationObject = {
  enabled: true,
  title: '',
  severity: '',
  organization: '',
  deadline: { value: '', unit: 'minute' },
  aliases: [
    {
      alias: '',
      uri: '',
      type: 'number',
      id: '0',
    },
  ],
  conditions: [],
  templates: [],
};

export const checkIfTemplateFieldsArePresent = (templates?: Template[]) => {
  if (templates && templates.length > 0) {
    const templateFieldsMissing = templates.some((template) => {
      if (template.type === 'list') {
        if (
          !template.token
          || !template.list
          || template.list.length < 0
          || !template.list.join(',')
        ) return true;
      }
      if (template.type === 'sequential') {
        if (!template.token || !template.to || !template.from) return true;
      }
      if (template.type === 'range') {
        if (
          !template.token
          || !template.range
          || template.range.length < 0
          || !template.range.join(',')
        ) return true;
      }
      return false;
    });
    return !templateFieldsMissing;
  }
  return true;
};

export const checkIfAliasesFieldsArePresent = (aliases?: Alias[], templates?: Template[]) => {
  if (aliases && aliases.length > 0) {
    const uniqueness = aliases.some((alias) => checkForUniqueness(alias, aliases, templates || []));
    const aliasFieldsMissing = aliases.some((alias) => !alias.alias || !alias.uri);
    return !aliasFieldsMissing && !uniqueness;
  }
  return false;
};

export const checkIfRuleLogicFieldsArePresent = (conditions?: Expression[]) => {
  if (conditions && conditions.length > 0) {
    const ruleLogicFieldsMissing = conditions.some(
      (condition) => condition.comparator1.value === undefined
        || !condition.conditional
        || !condition.comparator2.value
        || !condition.message,
    );
    return !ruleLogicFieldsMissing;
  }
  return false;
};

export const checkRequiredAlertValues = (alertValues: AlertConfigurationObject): boolean => alertValues.aliases.length === 0
  || !alertValues.deadline.unit
  || !alertValues.deadline.value
  || !alertValues.deadline.unit
  || !checkIfTemplateFieldsArePresent(alertValues.templates)
  || !checkIfAliasesFieldsArePresent(alertValues.aliases, alertValues.templates)
  || !checkIfRuleLogicFieldsArePresent(alertValues.conditions)
  || alertValues.enabled === undefined
  || alertValues.severity === ''
  || alertValues.severity === undefined
  || !alertValues.title
  || !alertValues.organization;

export const determineAlertManagementHeader = (
  alertManagementView: 'table' | 'form',
  alertFormValues?: AlertConfigurationObject | null,
) => {
  if (alertManagementView === 'table') return 'Alert Management';
  if (alertFormValues?.id) return 'Edit Alert';
  return 'Create New Alert';
};

export const alertManagementColumns = (): Column[] => {
  const arrayWithBools = [
    {
      id: 'deliver',
      label: 'Deliver',
    },
    {
      id: 'name',
      label: 'Alert Name',
    },
    {
      id: 'organization',
      label: 'Organization',
    },
    {
      id: 'severity',
      label: 'Severity',
    },
    {
      id: 'last_triggered',
      label: 'Last Triggered',
    },
    {
      id: 'deadline',
      label: 'Required Response Time',
    },
    {
      id: 'actions',
      label: 'Actions',
    },
  ];
  return arrayWithBools.filter(
    (item) => typeof item !== 'boolean' && item !== undefined,
  ) as Column[];
};
