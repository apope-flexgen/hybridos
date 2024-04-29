import { Column } from '@flexgen/storybook';
import { FLEET_MANAGER } from 'src/components/BaseApp';
import {
  AlertConfigurationObject, Alias, Expression, Template,
} from 'src/pages/ActivityLog/activityLog.types';

export type AlertManagementPageLayout = 'table' | 'form';

export const AlertManagementPageLayouts: {
  [key: string]: AlertManagementPageLayout
} = {
  TABLE: 'table',
  FORM: 'form',
};

export const generateInitialAliases = (aliases: Alias[]): Alias[] => [{
  alias: '', uri: '', type: 'number', id: aliases ? `${aliases.length + 1}` : '0',
}];

export const generateInitialTemplates = (templates: Template[]): Template[] => [{
  token: '', list: [], type: 'sequential', separateAlerts: false, id: templates ? `${templates.length + 1}` : '0',
}];

export const initialAlertManagementFilters = {
  limit: 50,
  page: 0,
  order: -1,
  orderBy: 'lastUpdated',
};

export const severities = [
  'Critical', 'High', 'Medium', 'Low',
];

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
  return (Number(value) * 60);
};

export const initialNewAlert: AlertConfigurationObject = {
  enabled: true,
  title: '',
  severity: '',
  organization: '',
  deadline: { value: '', unit: 'minute' },
  aliases: [{
    alias: '', uri: '', type: 'number', id: '0',
  }],
  conditions: [],
};

export const checkIfTemplateFieldsArePresent = (
  templates?: Template[],
) => {
  if (templates && templates.length > 0) {
    let fieldsMissing = false;
    templates.forEach((template) => {
      if (template.type === 'list') {
        fieldsMissing = !template.token || !template.list || template.list.length < 0 || !template.list.join(',');
      } else if (template.type === 'sequential') fieldsMissing = !template.token || !template.to || !template.from;
    });
    return !fieldsMissing;
  }
  return true;
};

export const checkIfAliasesFieldsArePresent = (
  aliases?: Alias[],
) => {
  if (aliases && aliases.length > 0) {
    const aliasFieldsMissing = aliases.some((alias) => !alias.alias || !alias.uri);
    return !aliasFieldsMissing;
  }
  return false;
};

export const checkIfRuleLogicFieldsArePresent = (
  conditions?: Expression[],
) => {
  if (conditions && conditions.length > 0) {
    const ruleLogicFieldsMissing = conditions.some(
      (condition) => (
        condition.comparator1.value === undefined
      || !condition.conditional
      || !condition.comparator2.value
      || !condition.message
      ),
    );
    return !ruleLogicFieldsMissing;
  }
  return false;
};

export const checkRequiredAlertValues = (
  alertValues: AlertConfigurationObject,
  product: string | null,
): boolean => (
  alertValues.aliases.length === 0
    || !alertValues.deadline.unit
    || !alertValues.deadline.value
    || !alertValues.deadline.unit
    || !checkIfTemplateFieldsArePresent(alertValues.templates)
    || !checkIfAliasesFieldsArePresent(alertValues.aliases)
    || !checkIfRuleLogicFieldsArePresent(alertValues.conditions)
    || alertValues.enabled === undefined
    || alertValues.severity === undefined
    || !alertValues.title
    || (
      product === FLEET_MANAGER
      && (
        !alertValues.organization
      )
    )
);

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
      id: 'deliver', label: 'Deliver',
    },
    {
      id: 'name', label: 'Alert Name',
    },
    {
      id: 'organization', label: 'Organization',
    },
    {
      id: 'severity', label: 'Severity',
    },
    {
      id: 'last_triggered', label: 'Last Triggered',
    },
    {
      id: 'deadline', label: 'Required Response Time',
    },
    {
      id: 'actions', label: 'Actions',
    },
  ];
  return arrayWithBools.filter((item) => typeof item !== 'boolean' && item !== undefined) as Column[];
};
