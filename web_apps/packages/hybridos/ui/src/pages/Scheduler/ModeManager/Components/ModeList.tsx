import {
  MuiButton, IconList, ThemeType, Typography,
} from '@flexgen/storybook';

import { Box } from '@mui/system';
import React from 'react';
import { ApiMode } from 'shared/types/dtos/scheduler.dto';
import { schedulerConfigLabels as labels } from 'src/pages/Scheduler/ModeManager/Helpers';
import { modeListBoxSx, modeListStyles as styles } from 'src/pages/Scheduler/ModeManager/Styles';
import { CustomColorType } from 'src/pages/Scheduler/ModeManager/Types';
import { useTheme } from 'styled-components';
import SiteInformation from './SiteInformation';

interface ModeListProps {
  modes: ApiMode | undefined | null;
  handleNavigation: (mode: any) => void;
  selectedModeId?: string | null;
  addMode?: () => void;
  schedulerType: 'SC' | 'FM' | null;
  siteName: string | undefined;
  disableModeManager: boolean;
}

const ModeList: React.FC<ModeListProps> = ({
  modes,
  handleNavigation,
  selectedModeId,
  addMode,
  siteName,
  schedulerType,
  disableModeManager,
}: ModeListProps) => {
  const theme = useTheme() as ThemeType;
  const boxStyles = modeListBoxSx(theme);

  return (
    <Box sx={boxStyles}>
      <SiteInformation schedulerType={schedulerType} siteName={siteName} />
      <Typography sx={styles.modesText} variant="bodyLBold" text="Modes" />
      {modes
        && Object.keys(modes).map((modeId: string) => (
          <MuiButton
            customColor={modes[modeId].color_code as CustomColorType}
            fullWidth
            label={modes[modeId].name}
            onClick={() => handleNavigation(modeId)}
            selected={selectedModeId ? selectedModeId === modeId : false}
            size="large"
            // FIXME: if the icon for the mode is not in iconlist this will break. should add a check
            startIcon={modes[modeId].icon as IconList}
            variant="outlined"
          />
        ))}
      <MuiButton
        disabled={disableModeManager}
        fullWidth
        label={labels.modeList.addButton.label}
        onClick={addMode}
        size="large"
        startIcon="Add"
        variant="outlined"
      />
    </Box>
  );
};

export default ModeList;
