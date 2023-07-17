/* eslint-disable */
// TODO: fix lint
import { ThemeType, MuiButton, Typography } from '@flexgen/storybook';
import {
  Table,
  TableBody,
  TableCell,
  TableContainer,
  TableFooter,
  TableHead,
  Paper,
} from '@mui/material';
import React, { useEffect, useState } from 'react';
import { validateURI } from 'shared/functions/uriValidation';
import { Setpoints, CreateButtonLabel } from 'src/pages/Scheduler/ModeManager/Types';
import { useTheme } from 'styled-components';
import SetpointRow from './SetpointRow';
import {
  expandingTableStyles as styles,
  expandingTableHeaderSx,
} from 'src/pages/Scheduler/ModeManager/Styles';

export interface ExpandingTableProps {
  type?: Setpoints;
  setpoint: any;
  updateSetpoint: any;
  addSetpoint: (type: any) => void;
  setSetpointError: (newValue: boolean) => void;
  disableModeManager: boolean;
}

const ExpandingTable: React.FC<ExpandingTableProps> = ({
  type = 'variables',
  setpoint,
  updateSetpoint,
  addSetpoint,
  setSetpointError,
  disableModeManager,
}: ExpandingTableProps) => {
  const theme = useTheme() as ThemeType;
  const headerStyles = expandingTableHeaderSx(theme);
  const [duplicateName, setDuplicateName] = useState<boolean>(false);

  useEffect(() => {
    const nameSet = new Set<string>();

    if (setpoint) {
      for (const obj of setpoint) {
        if (nameSet.has(obj.name.toLowerCase())) setDuplicateName(true);
        else setDuplicateName(false);
        nameSet.add(obj.name.toLowerCase());
        if (
          !obj.name ||
          !obj.uri ||
          !obj.type ||
          obj.value === undefined ||
          obj.value === '' ||
          !validateURI(obj.uri) ||
          duplicateName
        )
        setSetpointError(true);
        else if (!duplicateName && validateURI(obj.uri)) setSetpointError(false);
      }
    }
  }, [setpoint]);

  return (
    <TableContainer component={Paper} sx={styles.container}>
      <Table>
        <TableHead style={headerStyles}>
          <TableCell>
            <Typography
              variant='bodyLBold'
              text={setpoint?.length > 0 ? `Mode ${type}` : `No ${type} yet`}
            />
          </TableCell>
          <TableCell />
        </TableHead>
        <TableBody>
          {setpoint?.map((data: any) => (
            <SetpointRow
              disableModeManager={disableModeManager}
              duplicateName={duplicateName}
              key={data.id}
              setpoint={data}
              type={type}
              updateSetpoint={updateSetpoint}
            />
          ))}
        </TableBody>
        <TableFooter sx={styles.footer.main}>
          <TableCell sx={styles.footer.cell}>
            <MuiButton
              disabled={disableModeManager}
              color='primary'
              label={CreateButtonLabel(type)}
              onClick={() => addSetpoint(type)}
              startIcon='Add'
              variant='text'
            />
          </TableCell>
        </TableFooter>
      </Table>
    </TableContainer>
  );
};

export default ExpandingTable;
