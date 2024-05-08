import { MuiButton, Select, TextField } from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';

import {
  actionsBoxSx,
  setWidth,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import { Expression } from 'src/pages/ActivityLog/activityLog.types';

export interface Comparator2RowProps {
  expression: Expression;
  allExpressions: Expression[];
  comparatorOptions: string[];
  handleFieldChange: (field: string, updatedValue: any, commaSeparatedList?: boolean) => void;
}

const Comparator2Row = ({
  expression,
  allExpressions,
  comparatorOptions,
  handleFieldChange,
}: Comparator2RowProps) => {
  const handleComparator2FieldChange = (field: string, newValue: string) => {
    const expressionsWithoutCurrent = allExpressions.filter(
      (existingExpression) => expression.index !== existingExpression.index,
    );
    const newExpression: Expression = {
      ...expression,
      comparator2: { ...expression.comparator2, [field]: newValue },
    };
    if (field) handleFieldChange('conditions', [...expressionsWithoutCurrent, newExpression]);
  };

  const handleAddDuration = () => {
    const expressionsWithoutCurrent = allExpressions.filter(
      (existingExpression) => expression.index !== existingExpression.index,
    );
    const newExpression: Expression = {
      ...expression,
      duration: { value: '', unit: 'minute' },
    };
    handleFieldChange('conditions', [...expressionsWithoutCurrent, newExpression]);
  };
  return (
    <Box sx={actionsBoxSx}>
      {expression.comparator2.type === 'alias' ? (
        <Select
          required
          minWidth={190}
          menuItems={comparatorOptions}
          value={expression.comparator2.value.toString()}
          label="Alias"
          onChange={(e) => handleComparator2FieldChange('value', e.target.value)}
        />
      ) : (
        <Box sx={setWidth(190)}>
          <TextField
            required
            value={expression.comparator2.value.toString()}
            onChange={(e) => handleComparator2FieldChange('value', e.target.value)}
            label="Literal Value"
            fullWidth
          />
        </Box>
      )}
      <MuiButton variant="text" label="Add Duration" startIcon="Add" onClick={handleAddDuration} />
    </Box>
  );
};
export default Comparator2Row;
