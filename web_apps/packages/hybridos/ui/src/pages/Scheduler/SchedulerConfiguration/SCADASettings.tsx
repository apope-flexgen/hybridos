import { ThemeType, TextField } from '@flexgen/storybook';
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
  const updateField = (field: string, event: any) => {
    const newValue = event.target.value;
    setConfigEdits((prevState: any) => ({
      ...prevState,
      scada: {
        ...prevState.scada,
        [field]: Number(newValue),
      },
    }));
  };

  return (
    <Box sx={sx.box}>
      <Box sx={sx.boxRow}>
        <TextField
          inputProps={{ min: 0 }}
          label={labels.SCADA.stageSize}
          onChange={(event) => updateField('stage_size', event)}
          size="small"
          type="number"
          value={settings?.stage_size?.toString() || ''}
        />
        <TextField
          inputProps={{ min: 0 }}
          label={labels.SCADA.maxEvents}
          onChange={(event) => updateField('max_num_events', event)}
          size="small"
          type="number"
          value={settings?.max_num_events?.toString() || ''}
        />
      </Box>
      <Box sx={sx.boxRow}>
        <TextField
          inputProps={{ min: 0 }}
          label={labels.SCADA.ints}
          onChange={(event) => updateField('num_ints', event)}
          size="small"
          type="number"
          value={settings?.num_ints?.toString() || ''}
        />
        <TextField
          inputProps={{ min: 0 }}
          label={labels.SCADA.floats}
          onChange={(event) => updateField('num_floats', event)}
          size="small"
          type="number"
          value={settings?.num_floats?.toString() || ''}
        />
      </Box>
      <Box sx={sx.boxRow}>
        <TextField
          inputProps={{ min: 0 }}
          label={labels.SCADA.booleans}
          onChange={(event) => updateField('num_bools', event)}
          size="small"
          type="number"
          value={settings?.num_bools?.toString() || ''}
        />
        <TextField
          inputProps={{ min: 0 }}
          label={labels.SCADA.strings}
          onChange={(event) => updateField('num_strings', event)}
          size="small"
          type="number"
          value={settings?.num_strings?.toString() || ''}
        />
      </Box>
    </Box>
  );
};

export default SCADASettings;
