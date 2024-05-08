/* eslint-disable */
// TODO: fix lint
import {
  MuiButton,
  CardRow,
  TextField,
  Circle,
  SelectorContainer,
  IconButton,
  IconList,
  Typography,
} from '@flexgen/storybook';
import { Tooltip } from '@mui/material';
import { Box } from '@mui/system';
import React, { useEffect, useState } from 'react';
import {
  icons,
  newType,
  handleIconColor,
  updateModeFields,
  schedulerConfigLabels as labels,
} from 'src/pages/Scheduler/ModeManager/Helpers';
import {
  customColorsArr,
  CustomColorType,
  Actions,
  SetpointActions,
  ModeBody,
} from 'src/pages/Scheduler/ModeManager/Types';
import ExpandingTable from './SetpointTable/ExpandingTable';
import ModeButtons from './ModeButtons';
import { modeInfoStyles as styles } from 'src/pages/Scheduler/ModeManager/Styles';

interface ModeInfoProps {
  selectedModeId: string | null;
  updateMode: (mode: ModeBody | undefined | null, action: Actions) => void;
  checkChanges: () => void;
  changes: boolean;
  selectedModeValues?: ModeBody | null;
  setuneditedSelectedModeValues: any;
  modes: any;
  modesFromAPI: any;
  setselectedModeValues: React.Dispatch<React.SetStateAction<any>>;
  setselectedModeId: (newValue: any) => void;
  checkName: (selectedModeValues: any, id: any) => void;
  nameMatch: boolean;
  disableModeManager: boolean;
  disableSave: boolean;
  setDisableSave: any;
}

const ModeInfo: React.FC<ModeInfoProps> = ({
  selectedModeId,
  updateMode,
  checkChanges,
  changes,
  selectedModeValues,
  setselectedModeId,
  modes,
  modesFromAPI,
  setselectedModeValues,
  setuneditedSelectedModeValues,
  checkName,
  nameMatch,
  disableModeManager,
  disableSave,
  setDisableSave,
}: ModeInfoProps) => {
  const [setpointError, setSetpointError] = useState<boolean>(false);
  const [duplicateURI, setDuplicateURI] = useState<boolean>(false);
  const [invalidName, setInvalidName] = useState<boolean>(false);

  useEffect(() => {
    if (selectedModeId) {
      setselectedModeValues(modes[selectedModeId as keyof typeof modes]);
      setuneditedSelectedModeValues(modes[selectedModeId as keyof typeof modes]);
    }
  }, [selectedModeId]);

  useEffect(() => {
    const disabled =
      !changes || nameMatch || disableModeManager || duplicateURI || setpointError || invalidName;
    setDisableSave(disabled);
  }, [changes, nameMatch, disableModeManager, duplicateURI, setpointError, invalidName]);

  useEffect(() => {
    checkName(selectedModeValues, selectedModeId);
    checkChanges();
  }, [selectedModeValues, selectedModeId]);

  useEffect(() => {
    if (selectedModeValues) {
      const emptyName = !selectedModeValues.name || selectedModeValues.name.trim() === '';
      setInvalidName(emptyName);
      setDuplicateURI(checkDuplicateURIS(selectedModeValues));
    }
  }, [selectedModeValues]);

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

  function checkDuplicateURIS(mode: ModeBody): boolean {
    if (!modes) return false;
    const uris: Set<string> = new Set();

    for (const variable of mode.variables) {
      const uri = variable.uri.endsWith('/') ? variable.uri.slice(0, -1) : variable.uri;
      if (uris.has(uri.toLowerCase())) return true;
      uris.add(uri.toLowerCase());
    }

    for (const constant of mode.constants) {
      const uri = constant.uri.endsWith('/') ? constant.uri.slice(0, -1) : constant.uri;
      if (uris.has(uri.toLowerCase())) return true;
      uris.add(uri.toLowerCase());
    }

    return false;
  }

  const handleValueTypes = (type: string | undefined, value: string) => {
    if (type) {
      if (type === 'Int') {
        return parseInt(value, 10);
      }
      if (type === 'Float') {
        return parseFloat(value);
      }
      if (type === 'Bool') {
        if (value === 'true') {
          return true;
        }
        return false;
      }
    }
    return value;
  };

  const updateSetpoint = (
    id: string,
    type: keyof ModeBody,
    action: SetpointActions,
    property?: string,
    newValue?: any,
    setpointType?: string,
  ) => {
    if (selectedModeValues && selectedModeValues[type]) {
      // @ts-ignore
      const index = selectedModeValues[type].findIndex((variable: any) => variable.id === id);

      const arrayCopy: any[] = [...selectedModeValues[type]];

      if (action === 'update' && property) {
        if (index !== undefined) {
          switch (property) {
            case 'type': {
              arrayCopy[index] = {
                // @ts-ignore
                ...arrayCopy[index],
                [property]: newValue,
                value: '',
              };
              break;
            }
            case 'isTemplate': {
              arrayCopy[index] = {
                ...arrayCopy[index],
                batch_prefix: '',
                batch_range: null,
                batch_value: null,
              };
              break;
            }
            case 'batch_value':
            case 'batch_range': {
              arrayCopy[index] = {
                ...arrayCopy[index],
                [property]: newValue.split(',').map((v) => v.replace(' ', '')),
              };
              break;
            }
            default: {
              arrayCopy[index] = {
                ...arrayCopy[index],
                [property]:
                  property === 'value' ? handleValueTypes(setpointType, newValue) : newValue,
              };
            }
          }
        }
      } else if (action === 'delete') {
        arrayCopy.splice(index, 1);
        setSetpointError(false);
      }
      setselectedModeValues((prevState: any) => ({
        ...prevState,
        [type]: arrayCopy,
      }));
    }
  };

  const handleNameHelper = () => {
    if (invalidName) return labels.modeInfo.tooltip.emptyModeName;
    else if (nameMatch) return labels.modeInfo.tooltip.nameFieldHelperText;
    return undefined;
  };

  const defaultMode =
    selectedModeId?.toString().toLowerCase() === 'default' &&
    selectedModeValues?.name.toLowerCase() === 'default';

  return (
    <Box sx={styles.box}>
      <CardRow alignItems='center'>
        <Typography variant='headingS' text={labels.modeInfo.title.selectedModeId} />
        <ModeButtons
          selectedModeId={selectedModeId}
          selectedModeValues={selectedModeValues}
          updateMode={updateMode}
          modesFromAPI={modesFromAPI}
          disableModeManager={disableModeManager}
          disableSave={disableSave}
        />
      </CardRow>
      <CardRow alignItems='center'>
        <Typography variant='bodyLBold' text={selectedModeValues?.name || ''} />
        {selectedModeId?.toString().toLowerCase() === 'default' && (
          <Tooltip arrow title={labels.modeInfo.tooltip.defaultMode}>
            <div>
              <IconButton icon='Help' size='small' />
            </div>
          </Tooltip>
        )}
        {duplicateURI && (
          <Typography
            sx={styles.duplicateURITypo}
            variant='bodyM'
            text={labels.modeInfo.tooltip.duplicateURI}
            color='error'
          />
        )}
      </CardRow>
      <Box sx={{ direction: 'column' }}>
        <CardRow alignItems='center'>
          <Typography sx={styles.textWidth} variant='bodyL' text={labels.modeInfo.name} />
          <TextField
            disabled={disableModeManager || defaultMode}
            color={nameMatch || invalidName ? 'error' : 'primary'}
            helperText={handleNameHelper()}
            label={labels.modeInfo.name}
            onChange={(event: any) =>
              updateModeFields('name', undefined, setselectedModeValues, event)
            }
            size='small'
            value={selectedModeValues?.name}
          />
        </CardRow>
        <CardRow alignItems='center'>
          <Typography sx={styles.textWidth} variant='bodyL' text={labels.modeInfo.color} />
          <Box sx={styles.selectorWidth}>
            <SelectorContainer fullWidth value={selectedModeValues?.color_code} variant='color'>
              {customColorsArr.map((color: string) => (
                <Circle
                  color={color as CustomColorType}
                  onClick={() =>
                    !disableModeManager && updateModeFields('color', color, setselectedModeValues)
                  }
                  selected={selectedModeValues?.color_code === color}
                />
              ))}
            </SelectorContainer>
          </Box>
        </CardRow>
        <CardRow alignItems='center'>
          <Typography sx={styles.textWidth} variant='bodyL' text={labels.modeInfo.icon} />
          <Box sx={styles.selectorWidth}>
            <SelectorContainer
              disabled={disableModeManager}
              fullWidth
              value={selectedModeValues?.icon}
              variant='icon'
            >
              {icons.map((icon: string) => (
                <IconButton
                  color={handleIconColor(icon, selectedModeValues)}
                  icon={icon as IconList}
                  onClick={() => updateModeFields('icon', icon, setselectedModeValues)}
                />
              ))}
            </SelectorContainer>
          </Box>
        </CardRow>
        {selectedModeId?.toString().toLowerCase() === 'default' ? null : (
          <CardRow alignItems='start'>
            <Typography text='Variables' sx={styles.setpointText} variant='bodyL' />
            <ExpandingTable
              disableModeManager={disableModeManager}
              addSetpoint={addSetpoint}
              setSetpointError={setSetpointError}
              setpoint={selectedModeValues?.variables}
              type='variables'
              updateSetpoint={updateSetpoint}
            />
          </CardRow>
        )}
        <CardRow alignItems='start'>
          <Typography sx={styles.setpointText} variant='bodyL' text='Constants' />
          <ExpandingTable
            addSetpoint={addSetpoint}
            disableModeManager={disableModeManager}
            setSetpointError={setSetpointError}
            setpoint={selectedModeValues?.constants}
            type='constants'
            updateSetpoint={updateSetpoint}
          />
        </CardRow>
      </Box>
    </Box>
  );
};

export default ModeInfo;
