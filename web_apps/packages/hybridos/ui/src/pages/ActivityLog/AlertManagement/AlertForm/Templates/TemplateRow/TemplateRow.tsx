import {
  Accordion,

  MuiButton,
} from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';
import { useState } from 'react';
import ListFields from 'src/pages/ActivityLog/AlertManagement/AlertForm/Templates/TemplateRow/ListFields';

import SequentialFields from 'src/pages/ActivityLog/AlertManagement/AlertForm/Templates/TemplateRow/SequentialFields';
import TemplateInfo from 'src/pages/ActivityLog/AlertManagement/AlertForm/Templates/TemplateRow/TemplateInfo';

import {
  rightAlignedButton,
  setWidth,
  templateRowBoxSx,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import { Template } from 'src/pages/ActivityLog/activityLog.types';

export interface TemplateRowProps {
  allTemplates: Template[],
  template: Template,
  handleFieldChange: (field: string, updatedValue: any, commaSeparatedList?: boolean) => void,
  removeRow: (templateId: string) => void;
}

// Input fields for a single template - wildcard, list, and value
const TemplateRow = ({
  allTemplates,
  template,
  handleFieldChange,
  removeRow,
}: TemplateRowProps) => {
  const [expanded, setExpanded] = useState<boolean>(!template.token);
  const handleTemplateFieldChange = (
    field: string,
    newValue: string | string[] | boolean | number,
  ) => {
    const templatesWithoutCurrent = allTemplates.filter(
      (existingTemplate) => template.id !== existingTemplate.id,
    );
    if (field) handleFieldChange('templates', [...templatesWithoutCurrent, { ...template, [field]: newValue }]);
  };

  return (
    <Accordion
      heading={template.token}
      expanded={expanded}
      onChange={(isExpanded) => { setExpanded(isExpanded); }}
      accordionStyles={setWidth(650)}
    >
      <Box sx={{ ...templateRowBoxSx, flexDirection: 'column' }}>
        <TemplateInfo template={template} handleTemplateFieldChange={handleTemplateFieldChange} />
        {
        template.type === 'sequential'
          ? (
            <SequentialFields
              template={template}
              handleTemplateFieldChange={handleTemplateFieldChange}
            />
          )
          : (
            <ListFields
              template={template}
              handleTemplateFieldChange={handleTemplateFieldChange}
              range={template.type === 'range'}
            />
          )
      }
        {/*         <SeparateAlerts
          template={template}
          handleTemplateFieldChange={handleTemplateFieldChange}
        /> */}
        <Box sx={rightAlignedButton}>
          <MuiButton variant="text" startIcon="Close" color="error" label="Delete template" onClick={() => removeRow(template.id)} />
        </Box>
      </Box>
    </Accordion>
  );
};

export default TemplateRow;
