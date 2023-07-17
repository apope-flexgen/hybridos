/* eslint-disable */
// TODO: fix lint
import { CardContainer, ThemeType, Typography, EmptyContainer } from '@flexgen/storybook';
import Grid from '@mui/material/Grid';
import { ThemeProvider, Box } from '@mui/system';
import { NotifContextType, NotifContext } from 'src/contexts/NotifContext';
import isEqual from 'lodash.isequal';
import { useCallback, useEffect, useState, useContext } from 'react';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { useSchedulerContext } from 'src/pages/Scheduler/Scheduler';
import { schedulerURLS } from 'src/pages/Scheduler/SchedulerComponents/EventScheduler/EventSchedulerHelper';
import { ModalStateType } from 'src/pages/Scheduler/SchedulerComponents/Modal/Helpers';
import { useTheme } from 'styled-components';
import ModeInfo from './Components/ModeInfo';
import ModeList from './Components/ModeList';
import { newMode, schedulerConfigLabels as labels } from './Helpers';
import { createMuiTheme } from 'src/pages/Scheduler/ModeManager/Styles';

import { Actions, ModeBody } from './Types';

interface ModeManagerProps {
  schedulerType: 'SC' | 'FM' | null;
  setIsLoading: (newValue: boolean) => void;
}

const ModeManager: React.FC<ModeManagerProps> = ({
  schedulerType,
  setIsLoading,
}: ModeManagerProps) => {
  const theme = useTheme() as ThemeType;
  const muiTheme = createMuiTheme(theme);
  const [disableSave, setDisableSave] = useState<boolean>(false);
  const [changes, setChanges] = useState(false);
  const [nameMatch, setNameMatch] = useState(false);
  const [modesTemp, setModesTemp] = useState<any>(undefined);
  const [selectedModeId, setselectedModeId] = useState<string | null>(null);
  const [selectedModeValues, setselectedModeValues] = useState<ModeBody | null>(null);
  const [uneditedSelectedModeValues, setuneditedSelectedModeValues] = useState<ModeBody | null>(
    null,
  );
  const notifCtx = useContext<NotifContextType | null>(NotifContext);
  const axiosInstance = useAxiosWebUIInstance();

  const {
    siteName,
    modes,
    setUnsavedChange,
    navigationFlag,
    setModalOpen,
    setModalState,
    disableModeManager,
  } = useSchedulerContext();

  useEffect(() => {
    setModesTemp(modes);
  }, []);

  useEffect(() => {
    if (selectedModeValues === null) {
      setselectedModeId(null);
      setuneditedSelectedModeValues(null);
    }
  }, [selectedModeValues]);

  const updateModesTemp = () => {
    selectedModeId &&
      setModesTemp((prevState: any) => ({
        ...prevState,
        [selectedModeId]: selectedModeValues,
      }));
  };

  const addMode = async (mode: ModeBody | undefined | null, id: string | null) => {
    const newModes = id !== null && mode !== null ? { ...modes, [id]: mode } : modes;
    try {
      setIsLoading(true);
      if (id && modes[id]) await axiosInstance.post(`${schedulerURLS.addMode}/${id}`, mode);
      else if (id && !modes[id]) await axiosInstance.post(schedulerURLS.addMode, newModes);
      notifCtx?.notif('success', labels.notifications.saveSuccess);
    } finally {
      updateModesTemp();
      setChanges(false);
      setUnsavedChange(false);
      setIsLoading(false);
    }
  };

  const deleteMode = async (id: string | null) => {
    try {
      setIsLoading(true);
      await axiosInstance.delete(`${schedulerURLS.deleteMode}/${id}`);
      notifCtx?.notif('warning', labels.notifications.deleteSuccess);
    } finally {
      setIsLoading(false);
    }
  };

  useEffect(() => {
    if (disableModeManager) notifCtx?.notif('warning', labels.notifications.disabled);
  }, []);

  const checkChanges = useCallback(() => {
    if (selectedModeValues === null) {
      setChanges(false);
      setUnsavedChange(false);
      return;
    }
    const equal = isEqual(selectedModeValues, uneditedSelectedModeValues);
    const modeChanges = uneditedSelectedModeValues !== null && !equal;
    setChanges(modeChanges);
    setUnsavedChange(modeChanges);
  }, [selectedModeValues, uneditedSelectedModeValues, setUnsavedChange]);

  const checkName = (mode: any, id: any) => {
    if (!modesTemp) return;
    const match = Object.keys(modes).find(
      (modeId: string) =>
        modesTemp[modeId] &&
        modesTemp[modeId].name.toLowerCase() === mode?.name.toLowerCase() &&
        modeId !== id,
    );
    setNameMatch(!!match);
  };

  const updateMode = (mode: ModeBody | undefined | null, action: Actions, newMode?: boolean) => {
    if (!modes) return;
    if (action === 'save') {
      const modeVariablesDirty =
        selectedModeValues &&
        uneditedSelectedModeValues &&
        selectedModeValues.variables !== uneditedSelectedModeValues.variables;
      if (modeVariablesDirty) {
        setModalOpen(true);
        setModalState((prevState: ModalStateType) => ({
          ...prevState,
          type: 'addVariableToMode',
          secondaryActions: () => {
            setModalOpen(false);
            setselectedModeValues(
              selectedModeId ? modesTemp[selectedModeId as keyof typeof modes] : null,
            );
          },
          primaryActions: () => {
            setuneditedSelectedModeValues(selectedModeValues);
            addMode(mode, selectedModeId);
            setModalOpen(false);
          },
        }));
      } else {
        setuneditedSelectedModeValues(selectedModeValues);
        addMode(mode, selectedModeId);
      }
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
    } else if (action === 'cancel') {
      if (!selectedModeId) return;
      if (newMode) setselectedModeValues(modes[selectedModeId]);
      else {
        setselectedModeValues(null);
      }
    }
  };

  const populateUnsavedChangesModal = (modeId: string) => {
    setModalState((prevState: ModalStateType) => ({
      ...prevState,
      type: 'unsavedChanges',
      secondaryActions: () => {
        setModalOpen(false);
        setselectedModeValues(
          selectedModeId ? modesTemp[selectedModeId as keyof typeof modes] : null,
        );
        setselectedModeId(modeId);
      },
      primaryActions: undefined,
    }));
    if (!disableSave) {
      setModalState((prevState: ModalStateType) => ({
        ...prevState,
        type: 'unsavedChanges',
        secondaryActions: () => {
          setModalOpen(false);
          setselectedModeValues(
            selectedModeId ? modesTemp[selectedModeId as keyof typeof modes] : null,
          );
          setselectedModeId(modeId);
        },
        primaryActions: () => {
          setModalOpen(false);
          addMode(selectedModeValues, selectedModeId);
          setselectedModeId(modeId);
        },
      }));
    }
  };

  const handleNavigation = (modeId: string) => {
    if (!modesTemp) return;
    if (changes && selectedModeId !== modeId) {
      setModalOpen(true);
      populateUnsavedChangesModal(modeId);
    } else setselectedModeId(modeId);
  };

  useEffect(() => {
    if (navigationFlag) {
      checkChanges();
    }
  }, [navigationFlag, checkChanges]);

  const addNewMode = () => {
    const newApiMode = newMode();
    const id = newApiMode[0];
    const body = newApiMode[1];
    setModesTemp((prevState: any) => ({
      ...prevState,
      [id]: body,
    }));
    handleNavigation(newApiMode[0]);
  };

  return (
    <ThemeProvider theme={muiTheme}>
      <Grid container>
        <Grid item xs={3}>
          <ModeList
            addMode={addNewMode}
            handleNavigation={handleNavigation}
            modes={modes}
            schedulerType={schedulerType}
            selectedModeId={selectedModeId}
            siteName={siteName}
            disableModeManager={disableModeManager}
          />
        </Grid>
        <Grid item xs={9}>
          {selectedModeId ? (
            <ModeInfo
              changes={changes}
              checkChanges={checkChanges}
              checkName={checkName}
              modes={modesTemp}
              modesFromAPI={modes}
              nameMatch={nameMatch}
              selectedModeId={selectedModeId}
              selectedModeValues={selectedModeValues}
              setselectedModeId={setselectedModeId}
              setselectedModeValues={setselectedModeValues}
              setuneditedSelectedModeValues={setuneditedSelectedModeValues}
              updateMode={updateMode}
              disableModeManager={disableModeManager}
              disableSave={disableSave}
              setDisableSave={setDisableSave}
            />
          ) : (
            <EmptyContainer
              sx={{ minHeight: '500px' }}
              text={labels.modeInfo.title.noselectedModeId}
            />
          )}
        </Grid>
      </Grid>
    </ThemeProvider>
  );
};

export default ModeManager;
