import { ThemeType, MuiButton } from '@flexgen/storybook';
import {
  Typography,
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableFooter,
  TableHead,
  Paper,
} from '@mui/material';
import React from 'react';
import { headerSx } from 'src/pages/Scheduler/ModeManager/Helpers';
import { Setpoints, CreateButtonLabel } from 'src/pages/Scheduler/ModeManager/Types';
import { useTheme } from 'styled-components';
import SetpointRow from './SetpointRow';

export interface ExpandingTableProps {
  type?: Setpoints
  setpoint: any
  updateSetpoint: any
  addSetpoint: (type: any) => void
}

const ExpandingTable: React.FC<ExpandingTableProps> = ({
  type = 'variables',
  setpoint,
  updateSetpoint,
  addSetpoint,
}: ExpandingTableProps) => {
  const theme = useTheme() as ThemeType;
  const headerStyles = headerSx(theme);
  return (
    <TableContainer component={Paper} sx={{ width: '60%', overflow: 'none' }}>
      <Table>
        <TableHead style={setpoint?.length > 0 ? undefined : headerStyles}>
          <TableCell>
            <Typography variant="h3">
              {setpoint?.length > 0 ? `Mode ${type}` : `No ${type} yet`}
            </Typography>
          </TableCell>
          <TableCell />
        </TableHead>
        <TableBody>
          {setpoint?.map((data: any) => (
            <SetpointRow
              key={data.id}
              setpoint={data}
              type={type}
              updateSetpoint={updateSetpoint}
            />
          ))}
        </TableBody>
        <TableFooter sx={{ display: 'flex', alignItems: 'center' }}>
          <TableCell sx={{ borderBottom: 'none' }}>
            <MuiButton
              color="primary"
              label={CreateButtonLabel(type)}
              onClick={() => addSetpoint(type)}
              startIcon="Add"
              variant="text"
            />
          </TableCell>
        </TableFooter>
      </Table>
    </TableContainer>
  );
};

export default ExpandingTable;
