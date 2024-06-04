import {
  Typography, TextField, MuiButton, Select, ThemeType,
} from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';
import { useState, useEffect } from 'react';
import { useAlertFormContext } from 'src/pages/ActivityLog/AlertManagement/AlertForm/contexts/AlertFormContext';
import {
  alertManagementHelperText,
  generateExampleText,
  insertAtCursor,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.helpers';
import {
  accordionRowsSx,
  alertTemplateValueBox,
  displayFlex,
  messageBoxSx,
  messageFieldsBoxSx,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import { Expression } from 'src/pages/ActivityLog/activityLog.types';
import { useTheme } from 'styled-components';

export interface MessageRowProps {
  expression: Expression;
}

const MessageRow = ({ expression }: MessageRowProps) => {
  const theme = useTheme() as ThemeType;
  const { alertValues, handleFieldChange } = useAlertFormContext();
  const [selectedAlias, setSelectedAlias] = useState<string>(alertValues.aliases[0]?.alias || '');
  const [selectedTemplate, setSelectedTemplate] = useState<string>(
    alertValues.templates?.[0]?.token || '',
  );

  const handleMessageChange = (newValue: string, specialCharAddition?: boolean) => {
    const expressionsWithoutCurrent = alertValues.conditions.filter(
      (existingExpression) => expression.index !== existingExpression.index,
    );

    const field = document.getElementById('alert_message') as HTMLInputElement;

    const newExpression: Expression = {
      ...expression,
      message: specialCharAddition ? insertAtCursor(newValue, field) : newValue,
    };
    handleFieldChange('conditions', [...expressionsWithoutCurrent, newExpression]);
  };

  useEffect(() => {
    if (!selectedTemplate) setSelectedTemplate(alertValues.templates?.[0]?.token);
  }, [alertValues.templates]);

  return (
    <Box sx={messageBoxSx(theme)}>
      <Typography variant="bodySBold" text="Alert Message" />
      <Box sx={messageFieldsBoxSx}>
        <TextField
          required
          value={expression.message}
          onChange={(e) => handleMessageChange(e.target.value)}
          helperText="Max width 100"
          label=""
          multiline
          minRows={3}
          maxRows={3}
          fullWidth
          id="alert_message"
        />
        <Box sx={alertTemplateValueBox}>
          <Box sx={displayFlex}>
            <Select
              menuItems={alertValues.aliases.map((alias) => alias.alias) || []}
              label="Alias Value"
              disabled={alertValues.aliases.length === 0}
              minWidth={175}
              onChange={(e) => setSelectedAlias(e.target.value)}
              value={selectedAlias}
            />
            <MuiButton
              variant="text"
              disabled={!selectedAlias}
              label="Insert"
              startIcon="Add"
              onClick={() => handleMessageChange(`{${selectedAlias}}`, true)}
            />
          </Box>
          <Box sx={displayFlex}>
            <Select
              disabled={alertValues.templates?.length === 0}
              menuItems={alertValues.templates?.map((template) => template.token) || []}
              label="Template Value"
              minWidth={175}
              onChange={(e) => setSelectedTemplate(e.target.value)}
              value={selectedTemplate}
              variant="outlined"
            />
            <MuiButton
              disabled={!selectedTemplate}
              variant="text"
              label="Insert"
              startIcon="Add"
              onClick={() => handleMessageChange(`{${selectedTemplate}}`, true)}
            />
          </Box>
        </Box>
      </Box>
      <Box sx={accordionRowsSx}>
        <Typography variant="bodyS" text={alertManagementHelperText.expressionMessageFormatting} />
        <Typography
          variant="bodyS"
          text={alertManagementHelperText.expressionMessageFormattingTemplates}
        />
        <Typography
          variant="bodySBold"
          text={expression.message ? generateExampleText(alertValues, expression.message) : ''}
        />
      </Box>
    </Box>
  );
};
export default MessageRow;
