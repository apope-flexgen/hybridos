import { MuiButton } from '@flexgen/storybook';
import { Box } from '@mui/system';
import React from 'react';
import { schedulerConfigLabels as labels } from 'src/pages/Scheduler/ModeManager/Helpers';
import { modeInfoStyles as styles } from 'src/pages/Scheduler/ModeManager/Styles';
import { Actions, ModeBody } from 'src/pages/Scheduler/ModeManager/Types';

interface ModeButtonsProps {
  selectedModeId: string | null;
  updateMode: (mode: ModeBody | undefined | null, action: Actions, newMode?: boolean) => void;
  selectedModeValues: ModeBody | null | undefined;
  modesFromAPI: any;
  disableModeManager: boolean;
  disableSave: boolean;
}

const ModeButtons: React.FC<ModeButtonsProps> = ({
  disableModeManager,
  selectedModeId,
  modesFromAPI,
  disableSave,
  updateMode,
  selectedModeValues,
}: ModeButtonsProps) => {
  const newMode = !!Object.keys(modesFromAPI).find((name: string) => name === selectedModeId);

  return (
    <Box sx={styles.buttonBox}>
      <MuiButton
        disabled={disableModeManager}
        color="inherit"
        label={labels.modeInfo.buttons.cancel}
        onClick={() => updateMode(selectedModeValues, 'cancel', newMode)}
        size="medium"
        variant="text"
      />
      {selectedModeId?.toString().toLowerCase() !== 'default' && (
        <MuiButton
          disabled={disableModeManager || !newMode}
          color="error"
          label={labels.modeInfo.buttons.delete}
          onClick={() => updateMode(selectedModeValues, 'delete')}
          size="medium"
        />
      )}
      <MuiButton
        color="primary"
        disabled={disableSave}
        label={labels.modeInfo.buttons.save}
        onClick={() => updateMode(selectedModeValues, 'save')}
        size="medium"
      />
    </Box>
  );
};

export default ModeButtons;
