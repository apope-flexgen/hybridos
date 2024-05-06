import {
  Typography, Divider, MuiButton, Select,
} from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';
import { useAlertFormContext } from 'src/pages/ActivityLog/AlertManagement/AlertForm/contexts/AlertFormContext';
import { alertManagementHelperText } from 'src/pages/ActivityLog/AlertManagement/alertManagement.helpers';
import {
  formRowSx,
  formRowTitleAndDescriptionSx,
  formRowContentVerticalSx,
  externalFormRowBoxSx,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import { Organization } from 'src/pages/ActivityLog/activityLog.types';

export interface ScopeProps {
  setOrgModalOpen: React.Dispatch<React.SetStateAction<boolean>>;
  organizations: Organization[];
}
// Input fields for scope - sites and organization
const Scope = ({ setOrgModalOpen, organizations }: ScopeProps) => {
  const { alertValues, handleFieldChange } = useAlertFormContext();

  return (
    <Box sx={externalFormRowBoxSx}>
      <Box sx={formRowSx}>
        <Box sx={formRowTitleAndDescriptionSx}>
          <Typography text="Scope" variant="bodyMBold" />
          <Typography text={alertManagementHelperText.scope} variant="bodyM" />
        </Box>
        <Box sx={formRowContentVerticalSx}>
          <Select
            required
            minWidth={200}
            label="Organization"
            value={alertValues.organization || ''}
            onChange={(e) => handleFieldChange('organization', e.target.value)}
            menuItems={organizations.map((organization) => organization.name)}
          />
          <MuiButton
            label="Manage Organizations"
            variant="text"
            startIcon="EditOutlined"
            onClick={() => setOrgModalOpen(true)}
          />
        </Box>
      </Box>
      <Divider variant="fullWidth" orientation="horizontal" />
    </Box>
  );
};

export default Scope;
