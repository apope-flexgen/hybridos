import { MuiButton, IconList, ThemeType } from '@flexgen/storybook';
import { Typography, Tooltip } from '@mui/material';
import { Box } from '@mui/system';
import React from 'react';
import { ApiMode } from 'shared/types/dtos/scheduler.dto';
import { modeListBoxSx, schedulerConfigLabels as labels } from 'src/pages/Scheduler/ModeManager/Helpers';
import { CustomColorType } from 'src/pages/Scheduler/ModeManager/Types';
import { useTheme } from 'styled-components';
import SiteInformation from './SiteInformation';

interface ModeListProps {
  modes: ApiMode | undefined | null
  handleNavigation: (mode: any) => void
  selectedModeId?: string | null
  addMode?: () => void
  schedulerType: 'SC' | 'FM' | null
  siteName: string | undefined
}

const ModeList: React.FC<ModeListProps> = ({
  modes,
  handleNavigation,
  selectedModeId,
  addMode,
  siteName,
  schedulerType,
}: ModeListProps) => {
  const theme = useTheme() as ThemeType;
  const boxStyles = modeListBoxSx(theme);
  let newModeArray;
  if (modes) newModeArray = Object.keys(modes).map((modeId: string) => modes[modeId].name === 'New Mode');
  const isNewMode = newModeArray?.find((value: boolean) => value === true);

  return (
    <Box sx={boxStyles}>
      <SiteInformation schedulerType={schedulerType} siteName={siteName} />
      <Typography sx={{ marginTop: '20px' }} variant="h3">
        Modes
      </Typography>
      {modes && Object.keys(modes).map((modeId: string) => (
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
      <Tooltip arrow title={isNewMode ? labels.modeInfo.tooltip.addModeButton : ''}>
        <div>
          <MuiButton
            disabled={isNewMode}
            fullWidth
            label={labels.modeList.addButton.label}
            onClick={addMode}
            size="large"
            startIcon="Add"
            variant="outlined"
          />
        </div>
      </Tooltip>
    </Box>
  );
};

export default ModeList;
