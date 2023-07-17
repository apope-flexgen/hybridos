import {
  Box,
  Typography,
  IconButton,
  MuiButton,
  ThemeType,
  MaintModeModal,
} from '@flexgen/storybook';
import { Collapse } from '@mui/material';
import { useState } from 'react';
import { Roles } from 'shared/types/api/Users/Users.types';
import RealTimeService from 'src/services/RealTimeService/realtime.service';
import { useTheme } from 'styled-components';
import {
  getAllControlsBoxSx,
  getAllControlsHeaderBoxSx,
  getAllControlsInnerBoxSx,
} from './assetsPage.styles';

interface AllControlsProps {
  uris: string[];
  currentUser?: any;
}

const labels = {
  header: 'All Controls',
  maintModeHelper: 'Enabling Maintenance Mode here will affect all items',
  maintModeOn: 'Maintenance Mode On',
  maintModeOff: 'Maintenance Mode Off',
};

const AllControlsContainer = ({ uris, currentUser }: AllControlsProps) => {
  const theme = useTheme() as ThemeType;
  const [modalOpen, setModalOpen] = useState<boolean>(false);
  const [open, setOpen] = useState<boolean>(false);
  const realTimeService = RealTimeService.Instance;
  const { role } = currentUser;

  // TODO: find a different way to do this eventually
  const MAINT_MODE = '/maint_mode';

  const filteredURIs: string[] = uris
    .filter((uri: string) => !uri.includes('/summary'))
    .map((uri: string) => uri + MAINT_MODE);

  const turnOnMaintMode = () => {
    setModalOpen(true);
  };

  const turnOffMaintMode = () => {
    filteredURIs.forEach((uri: string) => {
      realTimeService.send('fimsNoReply', {
        method: 'set',
        uri,
        replyto: 'web_ui',
        body: false,
        username: 'web_ui',
      });
    });
  };

  const onModalSubmit = (reason: string, comment: string) => {
    // submit logging
    const data = {
      modified_field: 'maintenance_mode',
      modified_value: true,
      extraFields: {
        reason,
        comment,
      },
    };
    realTimeService.send('audit-logging', data);

    // turn on maint mode
    filteredURIs.forEach((uri: string) => {
      realTimeService.send('fimsNoReply', {
        method: 'set',
        uri,
        replyto: 'web_ui',
        body: true,
        username: 'web_ui',
      });
    });

    // close modal
    setModalOpen(false);
  };

  const controlsDisabled = role === Roles.Observer;

  return (
    <>
      <MaintModeModal
        open={modalOpen}
        onClose={() => setModalOpen(false)}
        onSubmit={onModalSubmit}
      />
      <Box sx={getAllControlsBoxSx(theme)}>
        <Box sx={getAllControlsHeaderBoxSx(theme)}>
          <Typography text={labels.header} variant="bodyLBold" />
          <IconButton icon={open ? 'ExpandLess' : 'ExpandMore'} onClick={() => setOpen(!open)} />
        </Box>
        <Collapse in={open}>
          <Box sx={getAllControlsInnerBoxSx(theme)}>
            <Typography text={labels.maintModeHelper} variant="helperText" />
            <MuiButton
              disabled={controlsDisabled}
              label={labels.maintModeOn}
              fullWidth
              onClick={turnOnMaintMode}
            />
            <MuiButton
              disabled={controlsDisabled}
              label={labels.maintModeOff}
              fullWidth
              onClick={turnOffMaintMode}
            />
          </Box>
        </Collapse>
      </Box>
    </>
  );
};

export default AllControlsContainer;
