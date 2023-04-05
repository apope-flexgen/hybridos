/* eslint-disable max-lines */
/* eslint-disable @typescript-eslint/ban-ts-comment */
import {
  MuiButton,
  CardRow,
  TextField,
  Circle,
  SelectorContainer,
  IconButton,
  IconList,
} from '@flexgen/storybook';
import { Tooltip, Typography } from '@mui/material';
import { Box } from '@mui/system';
import React, { useEffect } from 'react';
import {
  icons,
  newType,
  handleIconColor,
  updateModeFields,
  schedulerConfigLabels as labels,
  modeInfoSizing as sizes,
} from 'src/pages/Scheduler/ModeManager/Helpers';
import {
  customColorsArr,
  CustomColorType,
  Actions,
  SetpointActions,
  ModeBody,
} from 'src/pages/Scheduler/ModeManager/Types';
import ExpandingTable from './SetpointTable/ExpandingTable';

interface ModeInfoProps {
  selectedModeId: string | null
  updateMode: (mode: ModeBody | undefined | null, action: Actions) => void
  checkChanges: () => void
  changes: boolean
  selectedModeValues?: ModeBody | null
  setuneditedSelectedModeValues: any
  modes: any
  setselectedModeValues: React.Dispatch<React.SetStateAction<any>>
  checkName: (selectedModeValues: any, id: any) => void
  nameMatch: boolean
}

const ModeInfo: React.FC<ModeInfoProps> = ({
  selectedModeId,
  updateMode,
  checkChanges,
  changes,
  selectedModeValues,
  modes,
  setselectedModeValues,
  setuneditedSelectedModeValues,
  checkName,
  nameMatch,
}: ModeInfoProps) => {
  useEffect(() => {
    if (selectedModeId) {
      setselectedModeValues(modes[selectedModeId as keyof typeof modes]);
      setuneditedSelectedModeValues(modes[selectedModeId as keyof typeof modes]);
    }
  // TODO: Fix eslint-ignore
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [selectedModeId, setselectedModeValues, modes]);

  useEffect(() => {
    checkChanges();
  }, [selectedModeValues, checkChanges]);

  useEffect(() => {
    checkName(selectedModeValues, selectedModeId);
  }, [selectedModeValues, checkName, selectedModeId]);

  const addSetpoint = (type: keyof ModeBody) => {
    const newSetpoint = newType(type);
    if (!selectedModeValues) return;
    if (selectedModeValues[type]) {
      const tempArray = [...selectedModeValues[type], newSetpoint];
      setselectedModeValues((prevState: ModeBody) => ({
        ...prevState,
        [type]: tempArray,
      }));
    } else {
      const tempArray = [{}];
      tempArray[0] = newSetpoint;
      setselectedModeValues((prevState: ModeBody) => ({
        ...prevState,
        [type]: tempArray,
      }));
    }
  };

  const handleValueTypes = (
    type: string | undefined,
    value: string,
  ) => {
    if (type) {
      if (type === 'Int') { return parseInt(value, 10); }
      if (type === 'Float') { return parseFloat(value); }
      if (type === 'Bool') { if (value === 'true') { return true; } return false; }
    }
    return value;
  };

  const updateSetpoint = (
    id: string,
    type: keyof ModeBody,
    action: SetpointActions,
    property?: string,
    event?: any,
    setpointType?: string,
  ) => {
    if (selectedModeValues && selectedModeValues[type]) {
      // @ts-ignore
      const index = selectedModeValues[type].findIndex((variable: any) => variable.id === id);
      // @ts-ignore
      const arrayCopy = [...selectedModeValues[type]];
      if (action === 'update' && property) {
        if (index !== undefined) {
          arrayCopy[index] = {
            // @ts-ignore
            ...arrayCopy[index],
            [property]:
                            property === 'value'
                              ? handleValueTypes(setpointType, event.target.value)
                              : event.target.value,
          };
        }
      } else if (action === 'delete') {
        arrayCopy.splice(index, 1);
      }
      setselectedModeValues((prevState: any) => ({
        ...prevState,
        [type]: arrayCopy,
      }));
    }
  };

  const isNewMode = selectedModeValues?.name === 'New Mode';

  return (
    <Box sx={{ flexGrow: 1, padding: '16px' }}>
      <CardRow alignItems="center">
        <Typography variant="h1">{labels.modeInfo.title.selectedModeId}</Typography>
        <Box
          sx={{
            marginLeft: 'auto',
            display: 'flex',
            gap: '16px',
          }}
        >
          <MuiButton
            color="inherit"
            label={labels.modeInfo.buttons.cancel}
            onClick={() => selectedModeId && setselectedModeValues(modes[selectedModeId])}
            size="medium"
            variant="text"
          />
          {selectedModeId?.toString().toLowerCase() !== 'default' && (
            <MuiButton
              color="error"
              label={labels.modeInfo.buttons.delete}
              onClick={() => updateMode(selectedModeValues, 'delete')}
              size="medium"
            />
          )}
          <MuiButton
            color="primary"
            disabled={!changes || nameMatch}
            label={labels.modeInfo.buttons.save}
            onClick={() => updateMode(selectedModeValues, 'save')}
            size="medium"
          />
        </Box>
      </CardRow>
      <CardRow alignItems="center">
        <Typography variant="h3">{selectedModeValues?.name}</Typography>
        {selectedModeId?.toString().toLowerCase() === 'default' && (
        <Tooltip arrow title={labels.modeInfo.tooltip.defaultMode}>
          <div>
            <IconButton icon="Help" size="small" />
          </div>
        </Tooltip>
        )}
      </CardRow>
      <Box sx={{ direction: 'column' }}>
        <CardRow alignItems="center">
          <Typography sx={{ width: sizes.textWidth }} variant="body1">
            Name
          </Typography>
          <TextField
            disabled={selectedModeValues?.name.toLowerCase() === 'default'}
            color={nameMatch || isNewMode ? 'error' : 'primary'}
            helperText={
                            // eslint-disable-next-line no-nested-ternary
                            isNewMode
                              ? labels.modeInfo.tooltip.newModeNameHelperText
                              : nameMatch
                                ? labels.modeInfo.tooltip.nameFieldHelperText
                                : undefined
                        }
            label="Name"
            onChange={(event: any) => updateModeFields(
              'name',
              undefined,
              setselectedModeValues,
              event,
            )}
            size="small"
            value={selectedModeValues?.name}
          />
        </CardRow>
        <CardRow alignItems="center">
          <Typography sx={{ width: sizes.textWidth }} variant="body1">
            Color
          </Typography>
          <Box sx={{ minWidth: sizes.selectorWidth }}>
            <SelectorContainer
              fullWidth
              value={selectedModeValues?.color_code}
              variant="color"
            >
              {customColorsArr.map((color: string) => (
                <Circle
                  color={color as CustomColorType}
                  onClick={() => updateModeFields(
                    'color',
                    color,
                    setselectedModeValues,
                  )}
                  selected={selectedModeValues?.color_code === color}
                />
              ))}
            </SelectorContainer>
          </Box>
        </CardRow>
        <CardRow alignItems="center">
          <Typography sx={{ width: sizes.textWidth }} variant="body1">
            Icon
          </Typography>
          <Box sx={{ width: sizes.selectorWidth }}>
            <SelectorContainer
              fullWidth
              value={selectedModeValues?.icon}
              variant="icon"
            >
              {icons.map((icon: string) => (
                <IconButton
                  color={handleIconColor(icon, selectedModeValues)}
                  icon={icon as IconList}
                  onClick={() => updateModeFields(
                    'icon',
                    icon,
                    setselectedModeValues,
                  )}
                />
              ))}
            </SelectorContainer>
          </Box>
        </CardRow>
        {selectedModeId?.toString().toLowerCase() === 'default' ? null : (
          <CardRow alignItems="start">
            <Typography
              sx={{
                width: sizes.textWidth,
                marginTop: sizes.setpointMargin,
              }}
              variant="body1"
            >
              Variables
            </Typography>
            <ExpandingTable
              addSetpoint={addSetpoint}
              setpoint={selectedModeValues?.variables}
              type="variables"
              updateSetpoint={updateSetpoint}
            />
          </CardRow>
        )}
        <CardRow alignItems="start">
          <Typography
            sx={{
              width: sizes.textWidth,
              marginTop: sizes.setpointMargin,
            }}
            variant="body1"
          >
            Constants
          </Typography>
          <ExpandingTable
            addSetpoint={addSetpoint}
            setpoint={selectedModeValues?.constants}
            type="constants"
            updateSetpoint={updateSetpoint}
          />
        </CardRow>
      </Box>
    </Box>
  );
};

export default ModeInfo;
