import {
  MuiButton,
  Select,
} from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';
import { actionsBoxSx } from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import { logicalOperatorList } from 'src/pages/ActivityLog/activitiyLog.helpers';

import { Expression } from 'src/pages/ActivityLog/activityLog.types';

export interface Comparator1RowProps {
  comparatorOptions: string[],
  expression: Expression,
  allExpressions: Expression[],
  handleFieldChange: (field: string, updatedValue: any, commaSeparatedList?: boolean) => void,
}

const Comparator1Row = ({
  expression,
  comparatorOptions,
  allExpressions,
  handleFieldChange,
}: Comparator1RowProps) => {
  const handleComparator1FieldChange = (field: string, newValue: string) => {
    const expressionsWithoutCurrent = allExpressions.filter(
      (existingExpression) => expression.index !== existingExpression.index,
    );
    const newExpression: Expression = field === 'value'
      ? { ...expression, comparator1: { ...expression.comparator1, value: newValue } }
      : { ...expression, [field]: newValue };
    if (field) handleFieldChange('conditions', [...expressionsWithoutCurrent, newExpression]);
  };

  const handleAddComparator2 = (type: 'alias' | 'literal') => {
    const expressionsWithoutCurrent = allExpressions.filter(
      (existingExpression) => expression.index !== existingExpression.index,
    );
    const newExpression: Expression = { ...expression, comparator2: { type, value: '' } };
    handleFieldChange('conditions', [...expressionsWithoutCurrent, newExpression]);
  };
  return (
    <Box sx={actionsBoxSx}>
      {
        expression.connectionOperator
        && (
        <Select
          menuItems={['and', 'or']}
          value={expression.connectionOperator}
          minWidth={75}
          onChange={(e) => handleComparator1FieldChange('connectionOperator', e.target.value)}
        />
        )
    }
      <Select
        menuItems={comparatorOptions}
        value={expression.comparator1.value.toString()}
        label="Alias"
        onChange={(e) => handleComparator1FieldChange('value', e.target.value)}
      />
      <Select
        menuItems={logicalOperatorList}
        minWidth={75}
        label=""
        value={expression.conditional}
        onChange={(e) => handleComparator1FieldChange('conditional', e.target.value)}
      />
      <MuiButton variant="text" label="Add Alias" startIcon="Add" onClick={() => handleAddComparator2('alias')} />
      <MuiButton variant="text" label="Add Literal Value" startIcon="Add" onClick={() => handleAddComparator2('literal')} />
    </Box>
  );
};

export default Comparator1Row;
