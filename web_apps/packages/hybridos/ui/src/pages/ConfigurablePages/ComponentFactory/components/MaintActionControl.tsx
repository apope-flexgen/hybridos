import { Box, Select } from '@flexgen/storybook';
import { SelectChangeEvent } from '@mui/material';
import React, { useState } from 'react';
import RealTimeService from 'src/services/RealTimeService/realtime.service';
import ConfirmCancelButton from './ConfirmCancelButton';

export interface MaintActionControlProps {
  options: string[];
  disabled: boolean;
  controlURI: string;
}

const MaintActionControl: React.FC<MaintActionControlProps> = ({
  options,
  disabled,
  controlURI,
}: MaintActionControlProps) => {
  const [selectedAction, setSelectedAction] = useState<string>('');

  const handleOnClick = () => {
    const uriWithoutControl = controlURI.substring(0, controlURI.lastIndexOf('/'));
    const startActionURI = `${uriWithoutControl}/actions/${selectedAction}/start`;

    const realTimeService = RealTimeService.Instance;
    realTimeService.send('fimsNoReply', {
      method: 'set',
      uri: startActionURI,
      replyto: 'web_ui',
      body: true,
      username: 'web_ui',
    });

    setSelectedAction('');
  };

  const handleSelectAction = (event: SelectChangeEvent) => {
    setSelectedAction(event.target.value);
  };

  return (
    <Box sx={{ display: 'flex', flexDirection: 'column', gap: '4px' }}>
      <Select
        menuItems={options}
        label="Select Maintenance Action"
        fullWidth
        disabled={disabled}
        value={selectedAction}
        onChange={handleSelectAction}
      />
      <ConfirmCancelButton
        disabled={disabled || !selectedAction}
        label="Start Maintenance Action"
        onClick={handleOnClick}
        color="primary"
      />
    </Box>
  );
};

export default MaintActionControl;
