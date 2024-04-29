import {

  Select,

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

export interface TemplateInfoProps {
  template: Template,
  handleTemplateFieldChange:(
    field: string,
    newValue: string | string[] | boolean | number,
  ) => void,
}

// Input fields for a single template - wildcard, list, and value
const TemplateInfo = ({
  handleTemplateFieldChange,
  template,
}: TemplateInfoProps) => (
  <>
    <Box sx={formRowSx}>
      <Box sx={templateRowTitleAndDescriptionSx}>
        <Typography text="Wildcard Character" variant="bodyL" />
      </Box>
      <Box sx={templateFieldsBoxSx}>
        <TextField
          required
          label="Wildcard Character"
          value={template.token}
          onChange={(e) => handleTemplateFieldChange('token', e.target.value)}
        />
      </Box>
    </Box>
    <Box sx={formRowSx}>
      <Box sx={templateRowTitleAndDescriptionSx}>
        <Typography text="Template Type" variant="bodyL" />
      </Box>
      <Box sx={templateFieldsBoxSx}>
        <Select
          required
          minWidth={300}
          label="Template Type"
          menuItems={['list', 'sequential']}
          value={template.type}
          onChange={(e) => handleTemplateFieldChange('type', e.target.value)}
        />
      </Box>
    </Box>
  </>
);

export default TemplateInfo;
