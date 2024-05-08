import {
  IconButton, Select, TextField, Tooltip, Typography,
} from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';

import {
  durationRowSx,
  setWidth,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import { Expression } from 'src/pages/ActivityLog/activityLog.types';

export interface DurationRowProps {
  expression: Expression;
  allExpressions: Expression[];
  handleFieldChange: (field: string, updatedValue: any, commaSeparatedList?: boolean) => void;
}

const DurationRow = ({ expression, allExpressions, handleFieldChange }: DurationRowProps) => {
  const handleDurationFieldChange = (field: string, newValue: string | number) => {
    const expressionsWithoutCurrent = allExpressions.filter(
      (existingExpression) => expression.index !== existingExpression.index,
    );
    const newExpression: Expression = {
      ...expression,
      duration: expression.duration
        ? { ...expression.duration, [field]: newValue }
        : { unit: 'minute', value: '', [field]: newValue },
    };
    if (field) handleFieldChange('conditions', [...expressionsWithoutCurrent, newExpression]);
  };

  const removeDuration = () => {
    const expressionsWithoutCurrent = allExpressions.filter(
      (existingExpression) => expression.index !== existingExpression.index,
    );
    const tempExpression = expression;
    delete tempExpression.duration;
    handleFieldChange('conditions', [...expressionsWithoutCurrent, tempExpression]);
  };

  return (
    <Box sx={durationRowSx}>
      <Typography variant="buttonMedium" color="color" text="For more than" />
      <Box sx={setWidth(125)}>
        <TextField
          value={expression.duration?.value.toString() || ''}
          onChange={(e) => handleDurationFieldChange('value', e.target.value)}
          fullWidth
        />
      </Box>
      <Select
        value={`${expression.duration?.unit}(s)` || 'minute(s)'}
        menuItems={['hour(s)', 'minute(s)', 'second(s)']}
        onChange={(e) => handleDurationFieldChange('unit', e.target.value.replace('(s)', ''))}
      />
      <Tooltip title="Remove Duration" arrow position="right">
        <IconButton icon="Close" onClick={removeDuration} />
      </Tooltip>
    </Box>
  );
};
export default DurationRow;
