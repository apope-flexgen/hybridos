/* eslint-disable max-lines */
import {
  IconButton,
  MuiButton,
  Select,
  TextField,
  ThemeType,
  Typography,
} from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';
import { useState, useEffect } from 'react';
import { useAlertFormContext } from 'src/pages/ActivityLog/AlertManagement/AlertForm/contexts/AlertFormContext';
import {
  generateExampleText,
  insertAtCursor,
  checkForUniqueness,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.helpers';
import {
  aliasRowBoxSx,
  messageFieldsBoxSx,
  messageBoxSx,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import { Alias } from 'src/pages/ActivityLog/activityLog.types';
import { useTheme } from 'styled-components';

// Input fields for a single alias - alias, uri, and data type
export interface AliasRowProps {
  alias: Alias;
  removeRow: (aliasId: string) => void;
}
const AliasRow = ({ alias, removeRow }: AliasRowProps) => {
  const { alertValues, handleFieldChange } = useAlertFormContext();

  const [selectedTemplate, setSelectedTemplate] = useState<string>(
    alertValues.templates?.[0]?.token || '',
  );
  const [duplicateNameError, setDuplicateNameError] = useState<boolean>(false);
  const theme = useTheme() as ThemeType;

  useEffect(() => {
    if (!selectedTemplate) setSelectedTemplate(alertValues.templates?.[0]?.token || '');
  }, [alertValues.templates]);

  const handleAliasFieldChange = (field: string, newValue: string) => {
    const aliasesWithoutCurrent = alertValues.aliases.filter(
      (existingAlias) => alias.id !== existingAlias.id,
    );

    if (field) {
      handleFieldChange('aliases', [...aliasesWithoutCurrent, { ...alias, [field]: newValue }]);
    }
  };

  useEffect(() => {
    setDuplicateNameError(
      checkForUniqueness(alias, alertValues.aliases, alertValues.templates || []),
    );
  }, [alias.alias]);

  return (
    <Box sx={aliasRowBoxSx(theme)}>
      <Box sx={{ maxWidth: '200px' }}>
        <TextField
          fullWidth
          required
          label="Alias"
          value={alias.alias || ''}
          helperText={
            alias.alias && duplicateNameError
              ? 'Alias field must be unique and cannot overlap with a template wildcard'
              : ''
          }
          color={alias.alias && duplicateNameError ? 'error' : 'primary'}
          onChange={(e) => handleAliasFieldChange('alias', e.target.value)}
        />
      </Box>
      <Select
        required
        menuItems={['number', 'boolean', 'string']}
        label="Data Type"
        value={alias.type || 'number'}
        onChange={(e) => handleAliasFieldChange('type', e.target.value)}
      />
      <Box
        sx={alertValues?.templates && alertValues.templates?.length > 0 ? messageBoxSx(theme) : {}}
      >
        <Box sx={messageFieldsBoxSx}>
          <TextField
            required
            label="URI"
            id="alias_uri"
            TextAdornment="/"
            adornment="start"
            value={alias.uri || ''}
            onChange={(e) => handleAliasFieldChange('uri', e.target.value)}
          />
          {alertValues.templates && alertValues.templates.length > 0 && (
            <>
              <Select
                menuItems={alertValues.templates.map((template) => template.token) || []}
                label="Template Value"
                placeholder=" "
                minWidth={145}
                onChange={(e) => setSelectedTemplate(e.target.value)}
                value={selectedTemplate}
              />
              <MuiButton
                disabled={!selectedTemplate}
                variant="text"
                label="Insert"
                startIcon="Add"
                onClick={() => {
                  const field = document.getElementById('alias_uri') as HTMLInputElement;
                  const updatedValue = insertAtCursor(`{${selectedTemplate}}`, field);
                  handleAliasFieldChange('uri', updatedValue);
                }}
              />
            </>
          )}
        </Box>
        <Typography
          variant="bodySBold"
          text={
            alertValues.templates && alertValues.templates.length > 0 && alias.uri
              ? generateExampleText(alertValues, `/${alias.uri}`)
              : ''
          }
        />
      </Box>

      <IconButton icon="Close" onClick={() => removeRow(alias.id)} />
    </Box>
  );
};
export default AliasRow;
