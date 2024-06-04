import { Typography, Divider, MuiButton } from '@flexgen/storybook';
import Box from '@flexgen/storybook/dist/components/Atoms/Box/Box';

import { useState, useEffect } from 'react';
import TemplateRow from 'src/pages/ActivityLog/AlertManagement/AlertForm/Templates/TemplateRow/TemplateRow';
import { useAlertFormContext } from 'src/pages/ActivityLog/AlertManagement/AlertForm/contexts/AlertFormContext';
import {
  alertManagementHelperText,
  generateInitialTemplates,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.helpers';

import {
  formRowSx,
  formRowTitleAndDescriptionSx,
  externalFormRowBoxSx,
  templateFieldsBoxSx,
  accordionRowsSx,
} from 'src/pages/ActivityLog/AlertManagement/alertManagement.styles';
import { Template } from 'src/pages/ActivityLog/activityLog.types';

const Templates = () => {
  const { alertValues, handleFieldChange } = useAlertFormContext();

  const removeRow = (templateForRemoval: string) => {
    const newTemplates = alertValues.templates?.filter(
      (template: Template) => template.id !== templateForRemoval,
    );

    handleFieldChange('templates', newTemplates);
  };

  const addNewRow = () => {
    const initialTemplates = generateInitialTemplates(alertValues.templates || []);

    handleFieldChange(
      'templates',
      alertValues.templates ? [...alertValues.templates, ...initialTemplates] : initialTemplates,
    );
  };

  const generateTemplateRows = () => (alertValues.templates && alertValues.templates.length > 0
    ? alertValues.templates.sort((a, b) => (a.id || '').toString().localeCompare(b.id || '', undefined, { numeric: true }))
    : []
  ).map((template) => (
    <TemplateRow
      removeRow={removeRow}
      template={template}
      handleFieldChange={handleFieldChange}
      allTemplates={alertValues.templates || []}
    />
  ));

  const [templateRows, setTemplateRows] = useState<JSX.Element[]>(generateTemplateRows());

  useEffect(() => {
    setTemplateRows(generateTemplateRows());
  }, [alertValues.templates]);

  return (
    <Box sx={externalFormRowBoxSx}>
      <Box sx={formRowSx}>
        <Box sx={formRowTitleAndDescriptionSx}>
          <Typography text="Templates" variant="bodyMBold" />
          <Typography text={alertManagementHelperText.templates} variant="bodyM" />
        </Box>
        <Box sx={templateFieldsBoxSx}>
          <Box sx={accordionRowsSx}>{templateRows}</Box>
          <MuiButton variant="text" label="Add Template" startIcon="Add" onClick={addNewRow} />
        </Box>
      </Box>
      <Divider variant="fullWidth" orientation="horizontal" />
    </Box>
  );
};

export default Templates;
