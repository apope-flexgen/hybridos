import {
  Switch,
  Typography,
} from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';
import {
  templateRowTitleAndDescriptionSx,
  templateFieldsBoxSx,
  formRowSx,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import { Template } from 'src/pages/ActivityLog/activityLog.types';

export interface SeparateAlertsProps {
  template: Template,
  handleTemplateFieldChange:(
    field: string,
    newValue: string | string[] | boolean | number,
  ) => void,
}

// Input fields for a single template - wildcard, list, and value
const SeparateAlerts = ({
  handleTemplateFieldChange,
  template,
}: SeparateAlertsProps) => (
  <Box sx={formRowSx}>
    <Box sx={templateRowTitleAndDescriptionSx}>
      <Typography text="Separate Alerts" variant="bodyL" />
      <Typography text="If checked, you will receive a bespoke notification when conditions are met for each replacement value." variant="bodyS" />
    </Box>
    <Box sx={templateFieldsBoxSx}>
      <Switch
        value={template.padStart || false}
        onChange={(value) => handleTemplateFieldChange('padStart', value || false)}
      />
    </Box>
  </Box>
);

export default SeparateAlerts;
