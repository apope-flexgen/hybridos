import { Typography, TextField, Divider } from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';

import { useAlertFormContext } from 'src/pages/ActivityLog/AlertManagement/AlertForm/contexts/AlertFormContext';
import {
  formRowSx, formRowTitleAndDescriptionSx, formRowContentsSx, externalFormRowBoxSx,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';

// Input fields for scope - sites and organization
const Scope = () => {
  const { alertValues, handleFieldChange } = useAlertFormContext();

  return (
    <Box sx={externalFormRowBoxSx}>
      <Box sx={formRowSx}>
        <Box sx={formRowTitleAndDescriptionSx}>
          <Typography text="Scope" variant="bodyMBold" />
          <Typography text="Which organization should receive this alert" variant="bodyM" />
        </Box>
        <Box sx={formRowContentsSx}>
          <TextField required label="Organization" value={alertValues.organization} onChange={(e) => handleFieldChange('organization', e.target.value)} />
        </Box>
      </Box>
      <Divider variant="fullWidth" orientation="horizontal" />
    </Box>
  );
};

export default Scope;
