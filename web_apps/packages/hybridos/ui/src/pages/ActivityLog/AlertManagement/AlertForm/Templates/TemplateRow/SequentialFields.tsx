import {
  NumericInput,
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

export interface SequentialFieldsProps {
  template: Template,
  handleTemplateFieldChange:(
    field: string,
    newValue: string | string[] | boolean | number,
  ) => void,
}

// Input fields for a single template - wildcard, list, and value
const SequentialFields = ({
  handleTemplateFieldChange,
  template,
}: SequentialFieldsProps) => (
  <>
    <Box sx={formRowSx}>
      <Box sx={templateRowTitleAndDescriptionSx}>
        <Typography text="From" variant="bodyL" />
      </Box>
      <Box sx={templateFieldsBoxSx}>
        <NumericInput
          validationRegEx="positiveIntegers"
          required
          label="From"
          value={template.from?.toString() || ''}
          onChange={(e) => handleTemplateFieldChange('from', e.target.value)}
        />
      </Box>
    </Box>
    <Box sx={formRowSx}>
      <Box sx={templateRowTitleAndDescriptionSx}>
        <Typography text="To" variant="bodyL" />
      </Box>
      <Box sx={templateFieldsBoxSx}>
        <NumericInput
          validationRegEx="positiveIntegers"
          required
          label="To"
          value={template.to?.toString() || ''}
          onChange={(e) => handleTemplateFieldChange('to', e.target.value)}
        />
      </Box>
    </Box>
    <Box sx={formRowSx}>
      <Box sx={templateRowTitleAndDescriptionSx}>
        <Typography text="Pad Start" variant="bodyL" />
        <Typography text="If checked, 1-9 will be prefixed with a leading zero" variant="bodyS" />
      </Box>
      <Box sx={templateFieldsBoxSx}>
        <Switch
          value={template.separateAlerts || false}
          onChange={(value) => handleTemplateFieldChange('separateAlerts', value || false)}
        />
      </Box>
    </Box>
  </>
);

export default SequentialFields;
