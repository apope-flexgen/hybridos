import { ThemeType, NumericInput } from '@flexgen/storybook';
import { Box } from '@mui/system';
import React from 'react';
import { Configuration } from 'shared/types/dtos/scheduler.dto';
import { useTheme } from 'styled-components';
import { SCADASettingsSx, siteFleetConfigLabels as labels } from './Helpers';

interface SCADASettingsProps {
  settings: Configuration['scada']
  setConfigEdits: React.Dispatch<React.SetStateAction<Configuration | null | undefined>>
}

const SCADASettings: React.FC<SCADASettingsProps> = ({
  settings,
  setConfigEdits,
}: SCADASettingsProps) => {
  const theme = useTheme() as ThemeType;
  const sx = SCADASettingsSx(theme);
  const updateField = (field: string, value: any) => {
    setConfigEdits((prevState: any) => ({
      ...prevState,
      scada: {
        ...prevState.scada,
        [field]: Number(value),
      },
    }));
  };

  return (
    <Box sx={sx.box}>
      <Box sx={sx.boxRow}>
        <NumericInput
          label={labels.SCADA.stageSize}
          onChange={(event) => updateField('stage_size', event.target.value)}
          size="small"
          value={settings?.stage_size?.toString() || ''}
          validationRegEx="positiveIntegers"
        />
        <NumericInput
          label={labels.SCADA.maxEvents}
          onChange={(event) => updateField('max_num_events', event.target.value)}
          size="small"
          value={settings?.max_num_events?.toString() || ''}
          validationRegEx="positiveIntegers"
        />
      </Box>
      <Box sx={sx.boxRow}>
        <NumericInput
          label={labels.SCADA.ints}
          onChange={(event) => updateField('num_ints', event.target.value)}
          value={settings?.num_ints?.toString() || ''}
          validationRegEx="positiveIntegers"
        />
        <NumericInput
          label={labels.SCADA.floats}
          onChange={(event) => updateField('num_floats', event.target.value)}
          value={settings?.num_floats?.toString() || ''}
          validationRegEx="positiveIntegers"
        />
      </Box>
      <Box sx={sx.boxRow}>
        <NumericInput
          label={labels.SCADA.booleans}
          onChange={(event) => updateField('num_bools', event.target.value)}
          size="small"
          value={settings?.num_bools?.toString() || ''}
          validationRegEx="positiveIntegers"
        />
        <NumericInput
          label={labels.SCADA.strings}
          onChange={(event) => updateField('num_strings', event.target.value)}
          size="small"
          value={settings?.num_strings?.toString() || ''}
          validationRegEx="positiveIntegers"
        />
      </Box>
    </Box>
  );
};

export default SCADASettings;
