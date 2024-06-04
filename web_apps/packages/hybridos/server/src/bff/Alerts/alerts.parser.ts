import { AlertConditionalsMapping, LogicalOperators } from './alerts.constants';
import {
  AlertConfiguration,
  AlertConfigurationsResponse,
  Alias,
  Comparator,
  Deadline,
  Duration,
  Expression,
  Template,
} from './responses/alertConfig.response';

const parseDuration = (condition: string) => {
  const expressionWithoutParens = condition.split(/[\)\(]+/g)[1];
  return {
    duration: expressionWithoutParens.split(',')[1],
    restOfExpression: expressionWithoutParens.split(',')[0],
  };
};

const convertToSeconds = (value: number | string, unit: 'minute' | 'hour' | 'second') => {
  if (unit === 'minute') return Number(value) * 60;
  if (unit === 'hour') return Number(value) * 3600;
  return Number(value);
};

export const parseAliasForGoMetrics = (alias: string): string => {
  return alias.toString().replace(/ /g, '_');
};

const convertDurationToProperUnit = (value: string | number): Duration => {
  if (Number(value) % 3600 === 0) return { value: Number(value) / 3600, unit: 'hour' };
  if (Number(value) % 60 === 0) return { value: Number(value) / 60, unit: 'minute' };
  return { value, unit: 'second' };
};

export const parseAliasForUI = (alias: string): string => {
  const fragments = alias.split('_');
  return fragments.join(' ');
};

export const parseExpressionToConditions = (
  expression: string,
  aliases: Alias[],
  messages: string[],
): Expression[] => {
  if (expression) {
    const andOrRegEx = /[\||\&&]+/g;
    const listOfConditions = expression.split(andOrRegEx);
    const listOfConnectionOperators: ('or' | 'and')[] = [...expression.matchAll(andOrRegEx)].map(
      (match) => (match[0] === '||' ? 'or' : 'and'),
    );

    const conditions: Expression[] = listOfConditions.map((condition, index) => {
      let expressionWithoutDuration = condition;
      let durationSeconds = '';

      if (condition.includes('Duration')) {
        const { duration, restOfExpression } = parseDuration(condition);
        expressionWithoutDuration = restOfExpression;
        durationSeconds = duration;
      }

      const conditional =
        LogicalOperators.find((operator) => expressionWithoutDuration.includes(` ${operator} `)) ||
        '';

      const comparator1: Comparator = {
        type: 'alias',
        value: parseAliasForUI(expressionWithoutDuration.split(conditional)[0].trim()),
      };
      const comparator2Value = expressionWithoutDuration
        .split(conditional)[1]
        .trim()
        .replaceAll('"', '');
      const comparator2Type = aliases.some((alias) =>
        comparator2Value.includes(parseAliasForGoMetrics(alias.alias)),
      )
        ? 'alias'
        : 'literal';
      const comparator2: Comparator = {
        type: comparator2Type,
        value: comparator2Type === 'alias' ? parseAliasForUI(comparator2Value) : comparator2Value,
      };

      let connectionOperator = index !== 0 ? listOfConnectionOperators[index - 1] : null;

      const message = messages.find((message) => message[condition.trim()] !== undefined);
      let messageToSend = message[condition.trim()] || '';

      aliases.forEach((alias) => {
        if (message[condition.trim()].includes(`{${alias.alias}}`)) {
          messageToSend = message[condition.trim()].replaceAll(
            `{${alias.alias}}`,
            `{${parseAliasForUI(alias.alias)}}`,
          );
        }
      });

      const mappedCondition: Expression = {
        index,
        connectionOperator,
        comparator1,
        conditional,
        comparator2,
        duration: durationSeconds ? convertDurationToProperUnit(durationSeconds) : undefined,
        message: messageToSend,
      };

      return mappedCondition;
    });
    return conditions;
  }
  return [];
};

const parseComparatorLiteral = (value: string): string => {
  if (value.toLowerCase() === 'true' || value.toLowerCase() === 'false' || !isNaN(Number(value)))
    return value;
  return `"${value}"`;
};

export const parseConditionsToExpression = (
  conditions: Expression[],
  aliases: Alias[],
  templates?: Template[],
): { expression: string; messages: { [key: string]: string }[] } => {
  const sortedExpressions = conditions.sort((expression1, expression2) => {
    return expression1.index - expression2.index;
  });

  let expressionString = '';
  let messages: { [key: string]: string }[] = [];

  sortedExpressions.forEach((expression) => {
    const comparator1Value = parseAliasForGoMetrics(expression.comparator1.value.toString());

    const comparator2Value =
      expression.comparator2.type === 'alias'
        ? parseAliasForGoMetrics(expression.comparator2.value.toString())
        : parseComparatorLiteral(expression.comparator2.value.toString());

    const connectionOperator = expression.connectionOperator
      ? AlertConditionalsMapping[expression.connectionOperator]
      : '';

    let newExpression = `${comparator1Value} ${expression.conditional} ${comparator2Value}`;
    if (expression.duration) {
      const durationInSeconds = convertToSeconds(
        expression.duration.value,
        expression.duration.unit,
      );
      newExpression = `Duration(${newExpression}, ${durationInSeconds})`;
    }

    // if the user formatted values for aliases to be displayed in the message
    // parse the aliases correctly so go metrics can understand them
    // proper formatting -> {ESS 2 SOC} -> {ess_2_soc}
    const bracesRegex = /\{(.*?)\}/g;
    let formattedMessage = '';
    expression.message.split(bracesRegex).forEach((piece) => {
      const alias = aliases.find((alias: Alias) => `${alias.alias}` === piece);
      const template = templates.find((template: Template) => `${template.token}` === piece);
      if (alias) formattedMessage += `{${parseAliasForGoMetrics(alias.alias)}}`;
      else if (template) formattedMessage += `{${template.token}}`;
      else formattedMessage += `${piece}`;
    });

    messages.push({ [newExpression]: formattedMessage });

    newExpression = ` ${connectionOperator} ${newExpression}`;

    expressionString += newExpression;
  });

  return { expression: expressionString.trim(), messages };
};
