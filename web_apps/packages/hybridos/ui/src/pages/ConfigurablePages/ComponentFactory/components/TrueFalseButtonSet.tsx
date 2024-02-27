import { Box, MaintModeModal, MuiButton } from '@flexgen/storybook';
import React, { useState } from 'react';
import RealTimeService from 'src/services/RealTimeService/realtime.service';

export interface TrueFalseButtonSetProps {
  onClickHandlers: { [key: string]: ()=> void };
  label: string;
  disabled: boolean;
  isMaintMode: boolean;
}

const TrueFalseButtonSet: React.FC<TrueFalseButtonSetProps> = ({
  onClickHandlers,
  disabled,
  label,
  isMaintMode,
}: TrueFalseButtonSetProps) => {
  const defaultHandler = () => null;
  const [modalOpen, setModalOpen] = useState<boolean>(false);
  const realTimeService = RealTimeService.Instance;

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

    onClickHandlers?.true();
  };

  return (
    <Box sx={{ display: 'flex', flexDirection: 'column', gap: '8px' }}>
      {
        isMaintMode
        && (
        <MaintModeModal
          open={modalOpen}
          onClose={() => setModalOpen(false)}
          onSubmit={onModalSubmit}
        />
        )
      }
      <MuiButton
        label={`${label} - On`}
        fullWidth
        disabled={disabled}
        onClick={isMaintMode ? () => setModalOpen(true) : onClickHandlers?.true || defaultHandler}
      />
      <MuiButton
        disabled={disabled}
        label={`${label} - Off`}
        onClick={onClickHandlers?.false || defaultHandler}
      />
    </Box>
  );
};

export default TrueFalseButtonSet;
