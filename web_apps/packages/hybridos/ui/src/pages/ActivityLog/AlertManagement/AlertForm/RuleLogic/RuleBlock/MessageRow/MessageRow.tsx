import { Typography, TextField } from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';
import { alertManagementHelperText } from 'src/pages/ActivityLog/AlertManagement/alertManagement.helpers';
import { messageBoxSx } from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import { Expression } from 'src/pages/ActivityLog/activityLog.types';

export interface MessageRowProps {
  expression: Expression;
  allExpressions: Expression[];
  handleFieldChange: (field: string, updatedValue: any, commaSeparatedList?: boolean) => void;
}

const MessageRow = ({ expression, allExpressions, handleFieldChange }: MessageRowProps) => {
  const handleMessageChange = (newValue: string) => {
    const expressionsWithoutCurrent = allExpressions.filter(
      (existingExpression) => expression.index !== existingExpression.index,
    );
    const newExpression: Expression = {
      ...expression,
      message: newValue,
    };
    handleFieldChange('conditions', [...expressionsWithoutCurrent, newExpression]);
  };

  return (
    <Box sx={messageBoxSx}>
      <TextField
        required
        value={expression.message}
        onChange={(e) => handleMessageChange(e.target.value)}
        fullWidth
        label="Alert Message"
        multiline
        minRows={1}
        maxRows={3}
      />
      <Typography variant="bodyS" text={alertManagementHelperText.expressionMessageFormatting} />
      <Typography variant="bodyXSBold" text={alertManagementHelperText.expressionMessageExample} />
    </Box>
  );
};
export default MessageRow;
