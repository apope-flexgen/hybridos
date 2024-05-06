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

export const alertManagementHelperText = {
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
  expressionMessageExample:
    "If your alias were SOC Value, 'SOC reached {SOC Value}%' -> 'SOC reached 90%'",
  scope: 'Which organization should receive this alert',
  templates: 'Any templated wildcards to utilize in the aliases below',
  templateReplacementValues:
    'Enter a list of comma separated values to replace the wildcard character defined.',
  templateRangeReplacementValues: 'Ranges can be denoted with the following format - xx..xx.',
  templateRangeExample: '1..10 will become 1 through 10',
  templateWildcard: 'Wildcards should be contained in braces',
  templateWildcardExample: 'e.g. {ess}, {site}, {site_id}',
  templateSeparateAlerts:
    'If checked, you will receive a bespoke notification when conditions are met for each replacement value.',
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
    separateAlerts: false,
    id: templates ? `${templates.length + 1}` : '0',
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
      if (
        template.token.charAt(0) !== '{'
        || template.token.charAt(template.token.length - 1) !== '}'
      ) return true;
      return false;
    });
    return !templateFieldsMissing;
  }
  return true;
};

export const checkIfAliasesFieldsArePresent = (aliases?: Alias[]) => {
  if (aliases && aliases.length > 0) {
    const aliasFieldsMissing = aliases.some((alias) => !alias.alias || !alias.uri);
    return !aliasFieldsMissing;
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
  || !checkIfAliasesFieldsArePresent(alertValues.aliases)
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
