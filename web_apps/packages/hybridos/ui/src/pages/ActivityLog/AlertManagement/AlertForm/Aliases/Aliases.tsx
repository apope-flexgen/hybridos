import { Typography, Divider, MuiButton } from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';

import { useState, useEffect } from 'react';
import AliasRow from 'src/pages/ActivityLog/AlertManagement/AlertForm/Aliases/AliasRow/AliasRow';
import { useAlertFormContext } from 'src/pages/ActivityLog/AlertManagement/AlertForm/contexts/AlertFormContext';
import {
  generateInitialAliases,
  alertManagementHelperText,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.helpers';
import {
  formRowSx,
  formRowTitleAndDescriptionSx,
  externalFormRowBoxSx,
  templateFieldsBoxSx,
  expressionRowSx,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';

// Container for all aliases for the alert
const Aliases = () => {
  const { alertValues, handleFieldChange } = useAlertFormContext();

  const initialAliases = generateInitialAliases(alertValues.aliases);

  const removeRow = (aliasForRemoval: string) => {
    const newAliases = alertValues.aliases.filter((alias) => alias.id !== aliasForRemoval);
    handleFieldChange('aliases', newAliases);
  };

  const addNewRow = () => handleFieldChange(
    'aliases',
    alertValues.aliases ? [...alertValues.aliases, ...initialAliases] : initialAliases,
  );

  const generateAliasRows = () => (alertValues.aliases && alertValues.aliases.length > 0
    ? alertValues.aliases.sort((a, b) => a.id.toString().localeCompare(b.id, undefined, { numeric: true }))
    : []
  ).map((alias) => <AliasRow alias={alias} removeRow={removeRow} />);

  const [aliasRows, setAliasRows] = useState<JSX.Element[]>(generateAliasRows());

  useEffect(() => {
    setAliasRows(generateAliasRows());
  }, [alertValues.aliases]);

  return (
    <Box sx={externalFormRowBoxSx}>
      <Box sx={formRowSx}>
        <Box sx={formRowTitleAndDescriptionSx}>
          <Typography text="Aliases" variant="bodyMBold" />
          <Box sx={expressionRowSx}>
            <Typography text={alertManagementHelperText.aliases} variant="bodyM" />
            <Typography text={alertManagementHelperText.aliasWildcard} variant="bodyM" />
          </Box>
        </Box>
        <Box sx={templateFieldsBoxSx}>
          {aliasRows}
          <MuiButton variant="text" label="Add Alias" startIcon="Add" onClick={addNewRow} />
        </Box>
      </Box>
      <Divider variant="fullWidth" orientation="horizontal" />
    </Box>
  );
};

export default Aliases;
