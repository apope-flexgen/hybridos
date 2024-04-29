import {

  TextField,
  Typography,
} from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';
import {
  templateRowTitleAndDescriptionSx,
  templateFieldsBoxSx,
  formRowSx,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import { Template } from 'src/pages/ActivityLog/activityLog.types';

export interface ListFieldsProps {
  template: Template,
  handleTemplateFieldChange:(
    field: string,
    newValue: string | string[] | boolean | number,
  ) => void,
}

// Input fields for a single template - wildcard, list, and value
const ListFields = ({
  handleTemplateFieldChange,
  template,
}: ListFieldsProps) => (
  <Box sx={formRowSx}>
    <Box sx={templateRowTitleAndDescriptionSx}>
      <Typography text="Replacement Values" variant="bodyL" />
      <Typography text="Enter a list of comma separated values to replace the wildcard character defined." variant="bodyS" />
    </Box>
    <Box sx={templateFieldsBoxSx}>
      <TextField
        required
        label="Replacement Values"
        value={template.type === 'list' ? (template.list || []).join(',') : `${template.from}..${template.to}`}
        onChange={(e) => handleTemplateFieldChange('list', e.target.value.split(',').map((v) => v.replace(' ', '')))}
        helperText="Enter a list of comma separated values to replace the wildcard character defined."
      />
    </Box>
  </Box>
);

export default ListFields;
