// TODO: fix lint
/* eslint-disable max-statements, max-lines */
import { CardContainer, ThemeType } from '@flexgen/storybook';
import { Typography } from '@mui/material';
import { ThemeProvider, Box } from '@mui/system';
import isEqual from 'lodash.isequal';
import { useCallback, useEffect, useState } from 'react';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { useSchedulerContext } from 'src/pages/Scheduler/Scheduler';
import { schedulerURLS } from 'src/pages/Scheduler/SchedulerComponents/EventScheduler/EventSchedulerHelper';
import { ModalStateType } from 'src/pages/Scheduler/SchedulerComponents/Modal/Helpers';
import { noSelectedItemBoxSx } from 'src/pages/Scheduler/SchedulerHelpers';
import { useTheme } from 'styled-components';
import ModeInfo from './Components/ModeInfo';
import ModeList from './Components/ModeList';
import { createMuiTheme, newMode, schedulerConfigLabels as labels } from './Helpers';

import { Actions, ModeBody } from './Types';

interface ModeManagerProps {
  schedulerType: 'SC' | 'FM' | null
}

const ModeManager: React.FC<ModeManagerProps> = ({
  schedulerType,
}: ModeManagerProps) => {
  const theme = useTheme() as ThemeType;
  const muiTheme = createMuiTheme(theme);
  const [changes, setChanges] = useState(false);
  const [nameMatch, setNameMatch] = useState(false);
  const [selectedModeId, setselectedModeId] = useState<string | null>(null);
  const [selectedModeValues, setselectedModeValues] = useState<ModeBody | null>(null);
  const [uneditedSelectedModeValues, setuneditedSelectedModeValues] = useState<ModeBody | null>(
    null,
  );
  const axiosInstance = useAxiosWebUIInstance();

  const {
    siteName,
    modes,
    setUnsavedChange,
    navigationFlag,
    setModalOpen,
    setModalState,
  } = useSchedulerContext();

  const addMode = (mode: ModeBody | undefined | null, id: string | null) => {
    const newModes = (id !== null && mode !== null) ? { ...modes, [id]: mode } : modes;
    axiosInstance.post(schedulerURLS.addMode, newModes);
  };

  const deleteMode = (id: string | null) => {
    const index = Object.keys(modes).findIndex((modeId: string) => modeId === id);
    if (index !== -1 && id !== null) {
      delete modes[id];
      setUnsavedChange(false);
    }
    axiosInstance.post(schedulerURLS.deleteMode, modes);
  };

  const checkChanges = useCallback(() => {
    const equal = isEqual(selectedModeValues, uneditedSelectedModeValues);
    const modeChanges = uneditedSelectedModeValues !== null && !equal;
    setChanges(modeChanges);
    setUnsavedChange(modeChanges);
  }, [selectedModeValues, uneditedSelectedModeValues, setUnsavedChange]);

  const checkName = (mode: any, id: any) => {
    if (!modes) return;
    const match = Object.keys(modes).find(
      (modeId: string) => modes[modeId].name === mode?.name && modeId !== id,
    );
    setNameMatch(!!match);
  };

  const updateMode = (mode: ModeBody | undefined | null, action: Actions) => {
    if (!modes) return;
    if (action === 'save') {
      setuneditedSelectedModeValues(selectedModeValues);
      addMode(mode, selectedModeId);
    } else if (action === 'delete') {
      setModalOpen(true);
      setModalState((prevState: ModalStateType) => ({
        ...prevState,
        type: 'deleteConfirmation',
        secondaryActions: () => {
          setModalOpen(false);
        },
        primaryActions: () => {
          deleteMode(selectedModeId);
          setChanges(false);
          setModalOpen(false);
          setselectedModeId(null);
          setselectedModeValues(null);
          setuneditedSelectedModeValues(null);
        },
      }));
    }
  };

  const populateUnsavedChangesModal = (modeId: string) => {
    setModalState((prevState: ModalStateType) => ({
      ...prevState,
      type: 'unsavedChanges',
      secondaryActions: () => {
        setModalOpen(false);
        setselectedModeValues(
          selectedModeId ? modes[selectedModeId as keyof typeof modes] : null,
        );
        setselectedModeId(modeId);
      },
      primaryActions: undefined,
    }));
    if (!nameMatch) {
      setModalState((prevState: ModalStateType) => ({
        ...prevState,
        type: 'unsavedChanges',
        secondaryActions: () => {
          setModalOpen(false);
          setselectedModeValues(
            selectedModeId
              ? modes[selectedModeId as keyof typeof modes]
              : null,
          );
          setselectedModeId(modeId);
        },
        primaryActions: () => {
          setModalOpen(false);
          updateMode(selectedModeValues, 'save');
          setselectedModeId(modeId);
        },
      }));
    }
  };

  const handleNavigation = (modeId: string, isNewMode?: boolean) => {
    if (!modes) return;
    if (changes && selectedModeId !== modeId) {
      setModalOpen(true);
      populateUnsavedChangesModal(modeId);
    } else if (!isNewMode) setselectedModeId(modeId);
  };

  useEffect(() => {
    if (navigationFlag) {
      checkChanges();
    }
  }, [navigationFlag, checkChanges]);

  const addNewMode = () => {
    const newApiMode = newMode();
    addMode(newApiMode[1], newApiMode[0]);
    handleNavigation(newApiMode[0], true);
  };

  return (
    <ThemeProvider theme={muiTheme}>
      <CardContainer flexDirection="row">
        <ModeList
          addMode={addNewMode}
          handleNavigation={handleNavigation}
          modes={modes}
          schedulerType={schedulerType}
          selectedModeId={selectedModeId}
          siteName={siteName}
        />
        {selectedModeId ? (
          <ModeInfo
            changes={changes}
            checkChanges={checkChanges}
            checkName={checkName}
            modes={modes}
            nameMatch={nameMatch}
            selectedModeId={selectedModeId}
            selectedModeValues={selectedModeValues}
            setselectedModeValues={setselectedModeValues}
            setuneditedSelectedModeValues={setuneditedSelectedModeValues}
            updateMode={updateMode}
          />
        ) : (
          <Box sx={noSelectedItemBoxSx(theme)}>
            <Typography variant="h1">
              {labels.modeInfo.title.noselectedModeId}
            </Typography>
          </Box>
        )}
      </CardContainer>
    </ThemeProvider>
  );
};

export default ModeManager;
