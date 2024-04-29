import {
  Divider, IconButton, Tooltip,
} from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';

import Comparator1Row from 'src/pages/ActivityLog/AlertManagement/AlertForm/RuleLogic/RuleBlock/Comparator1Row/Comparator1Row';
import Comparator2Row from 'src/pages/ActivityLog/AlertManagement/AlertForm/RuleLogic/RuleBlock/Comparator2Row/Comparator2Row';
import DurationRow from 'src/pages/ActivityLog/AlertManagement/AlertForm/RuleLogic/RuleBlock/DurationRow/DurationRow';
import MessageRow from 'src/pages/ActivityLog/AlertManagement/AlertForm/RuleLogic/RuleBlock/MessageRow/MessageRow';

import { initialNewExpression } from 'src/pages/ActivityLog/AlertManagement/alertManagement.helpers';
import { comparator1RowSx, expressionRowSx } from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import { Expression, Alias } from 'src/pages/ActivityLog/activityLog.types';

export interface ExpressionBlockProps {
  expressions: Expression[],
  expression: Expression | null,
  aliases: Alias[],
  handleFieldChange: (field: string, updatedValue: any, commaSeparatedList?: boolean) => void,
}
const ExpressionBlock = ({
  expressions,
  expression,
  aliases,
  handleFieldChange,
}: ExpressionBlockProps) => {
  const aliasValues = aliases.map((alias) => alias.alias);

  const removeExpression = () => {
    const newExpressions = expressions.filter(
      (existingExpression) => existingExpression.index !== expression?.index,
    );
    handleFieldChange('conditions', newExpressions);
  };

  const expressionToUse = expression || initialNewExpression(expressions);
  return (
    <Box sx={expressionRowSx}>
      <Box sx={comparator1RowSx}>
        <Comparator1Row
          comparatorOptions={aliasValues}
          expression={expressionToUse}
          allExpressions={expressions}
          handleFieldChange={handleFieldChange}
        />
        {
          expressionToUse.index !== 0
          && (
          <Tooltip title="Remove Condition" arrow position="right">
            <IconButton icon="Close" onClick={removeExpression} />
          </Tooltip>
          )
        }
      </Box>
      <Comparator2Row
        comparatorOptions={aliasValues}
        expression={expressionToUse}
        allExpressions={expressions}
        handleFieldChange={handleFieldChange}
      />
      {
        expressionToUse.duration
          && (
          <DurationRow
            expression={expressionToUse}
            allExpressions={expressions}
            handleFieldChange={handleFieldChange}
          />
          )
      }
      <MessageRow
        expression={expressionToUse}
        allExpressions={expressions}
        handleFieldChange={handleFieldChange}
      />
      <Divider variant="fullWidth" orientation="horizontal" />
    </Box>
  );
};
export default ExpressionBlock;
