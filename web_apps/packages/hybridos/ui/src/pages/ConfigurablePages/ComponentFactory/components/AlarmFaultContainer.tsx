import { Chip, Typography } from '@flexgen/storybook';

export interface AlarmFaultContainerProps {
  showAlarm?: boolean;
  showFault?: boolean;
  tableView?: boolean;
}

const AlarmFaultContainer: React.FC<AlarmFaultContainerProps> = ({
  showAlarm = false,
  showFault = false,
  tableView = false,
}: AlarmFaultContainerProps) => {
  if (tableView) {
    const showDash = !showAlarm && !showFault;
    return (
      <>
        {showDash && <Typography text="-" />}
        {showAlarm && <Chip label="Alarm" icon="Alarm" color="warning" size="small" />}
        {showFault && <Chip label="Fault" icon="Fault" color="error" size="small" />}
      </>
    );
  }

  return (
    <div style={showAlarm || showFault ? { padding: '0.5rem 1rem' } : {}}>
      {showAlarm && <Chip label="Alarm" icon="Alarm" color="warning" size="small" />}
      {showFault && <Chip label="Fault" icon="Fault" color="error" size="small" />}
    </div>
  );
};

export default AlarmFaultContainer;
