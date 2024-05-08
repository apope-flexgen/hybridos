import {
  Divider, Select, TextField, Typography,
} from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';
import { useEffect, useState } from 'react';
import { alertManagementHelperText } from 'src/pages/ActivityLog/AlertManagement/alertManagement.helpers';

import {
  templateRowTitleAndDescriptionSx,
  templateFieldsBoxSx,
  formRowSx,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import { Template } from 'src/pages/ActivityLog/activityLog.types';

export interface TemplateInfoProps {
  template: Template;
  handleTemplateFieldChange: (
    field: string,
    newValue: string | string[] | boolean | number,
  ) => void;
}

// Input fields for a single template - wildcard, list, and value
const TemplateInfo = ({ handleTemplateFieldChange, template }: TemplateInfoProps) => {
  const [wildcardFormatError, setWildcardFormatError] = useState<boolean>(false);
  useEffect(() => {
    if (
      template.token.charAt(0) === '{'
      && template.token.charAt(template.token.length - 1) === '}'
    ) setWildcardFormatError(false);
    else setWildcardFormatError(true);
  }, [template]);
  return (
    <>
      <Box sx={formRowSx}>
        <Box sx={templateRowTitleAndDescriptionSx}>
          <Typography text="Wildcard" variant="bodyL" />
          <Typography text={alertManagementHelperText.templateWildcard} variant="bodyS" />
          <Typography
            text={alertManagementHelperText.templateWildcardExample}
            variant="bodySBold"
          />
        </Box>
        <Box sx={templateFieldsBoxSx}>
          <TextField
            required
            label="Wildcard Character"
            value={template.token}
            color={wildcardFormatError ? 'error' : 'primary'}
            helperText={wildcardFormatError ? alertManagementHelperText.templateWildcard : ''}
            onChange={(e) => handleTemplateFieldChange('token', e.target.value)}
          />
        </Box>
      </Box>
      <Divider variant="fullWidth" orientation="horizontal" />
      <Box sx={formRowSx}>
        <Box sx={templateRowTitleAndDescriptionSx}>
          <Typography text="Type" variant="bodyL" />
        </Box>
        <Box sx={templateFieldsBoxSx}>
          <Select
            required
            minWidth={300}
            label="Type"
            menuItems={['list', 'sequential', 'range']}
            value={template.type}
            onChange={(e) => handleTemplateFieldChange('type', e.target.value)}
          />
        </Box>
      </Box>
      <Divider variant="fullWidth" orientation="horizontal" />
    </>
  );
};

export default TemplateInfo;
