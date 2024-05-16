/* eslint-disable */
// TODO: fix lint
import { Typography, Icon, IconButton, ThemeType, MuiButton } from '@flexgen/storybook';
import { Box } from '@mui/material';
import { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { AlertState } from 'src/pages/ConfigurablePages/configurablePages.types';
import { useTheme } from 'styled-components';
import {
  FadeGradient,
  getOuterBoxSx,
  headerBoxSx,
  alertContainerSx,
  viewEventsButtonBoxSx,
  expandButtonBoxSx,
} from './assetsPage.styles';

type SeverityEnum = 'FAULT' | 'ALARM';

const ViewEventsButton = ({ severity }: { severity: string }) => {
  const navigate = useNavigate();

  const navigateToEvents = () => {
    navigate('/activity-log');
  };

  const color = severity === 'FAULT' ? 'error' : 'warning';

  return (
    <MuiButton
      variant='outlined'
      size='medium'
      endIcon='ArrowForward'
      label='VIEW'
      onClick={navigateToEvents}
      color={color}
    />
  );
};

const Header = ({ alertCount, severity }: { alertCount: number; severity: SeverityEnum }) => {
  const categoryText = alertCount > 1 ? `${severity}S` : `${severity}`;
  const numText = alertCount > 1 ? `${alertCount} ` : '';
  const headerText = `${numText}ACTIVE ${categoryText}`;
  const iconSrc = severity === 'FAULT' ? 'Fault' : 'Alarm';

  return (
    <>
      <Icon src={iconSrc} color={severity === 'FAULT' ? 'error' : 'warning'} />
      <Typography
        variant='bodyMBold'
        text={headerText}
        color={severity === 'FAULT' ? 'error' : 'warning'}
      />
      <Box sx={viewEventsButtonBoxSx}>
        <ViewEventsButton severity={severity} />
      </Box>
    </>
  );
};

const AlertBox = ({ alerts, severity }: { alerts: string[]; severity: SeverityEnum }) => {
  const theme = useTheme() as ThemeType;
  // if it is expandable, we want it to start closed
  // if it is not expandable, it should be considered "open"
  const [expanded, setExpanded] = useState(!(alerts.length > 3));
  const boxColorTransparent =
    severity === 'FAULT' ? `${theme.fgd.fault.main_0p}12` : `${theme.fgd.alarm.main_0p}12`;
  const boxColor = severity === 'FAULT' ? `${theme.fgd.fault.main}40` : `${theme.fgd.alarm.main}40`;
  const paperColorTransparent = theme.fgd.background.paper_0p;
  const paperColor = theme.fgd.background.paper;
  const onExpandClick = () => setExpanded((prev) => !prev);

  if (!alerts || alerts.length < 1) {
    return <></>;
  }

  const expandable = alerts.length > 3;

  const expandButton = (
    <Box sx={expandButtonBoxSx}>
      <IconButton icon={expanded ? 'ArrowDropUp' : 'ArrowDropDown'} color='primary' />
    </Box>
  );

  return (
    <Box sx={getOuterBoxSx(expandable, expanded, boxColor)} onClick={onExpandClick}>
      <Box sx={headerBoxSx}>
        <Header alertCount={alerts.length} severity={severity} />
      </Box>
      <ul>
        {alerts.map((alertString) => (
          <li style={{ color: theme.fgd.text.primary }}>
            <Typography variant='bodyS' text={alertString} />
          </li>
        ))}
      </ul>
      {!expanded && (
        <FadeGradient
          {...{
            boxColor,
            boxColorTransparent,
            paperColor,
            paperColorTransparent,
          }}
        />
      )}
      {expandable && expandButton}
    </Box>
  );
};

const AlertContainer = ({ alerts }: { alerts: AlertState[string] }) => (
  <Box sx={alertContainerSx}>
    <AlertBox alerts={alerts.faultInformation} severity='FAULT' />
    <AlertBox alerts={alerts.alarmInformation} severity='ALARM' />
  </Box>
);

export default AlertContainer;
