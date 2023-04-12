/* eslint-disable max-lines */
import {
  NumericInput, MuiButton, Select, TextField, IconButton,
} from '@flexgen/storybook';
import {
  Box,
  Typography,
  Collapse,
  Table,
  TableBody,
  TableCell,
  TableFooter,
  TableRow,
} from '@mui/material';
import React, { useEffect, useState } from 'react';
import { schedulerConfigLabels as labels } from 'src/pages/Scheduler/ModeManager/Helpers';
import { Setpoints, SetpointTypes } from 'src/pages/Scheduler/ModeManager/Types';

interface SetpointRowProps {
  setpoint?: any
  updateSetpoint?: any
  type?: Setpoints
}

const SetpointRow: React.FC<SetpointRowProps> = ({
  setpoint,
  updateSetpoint,
  type,
}: SetpointRowProps) => {
  const [open, setOpen] = useState(false);

  const handleValueComponent = () => {
    if (setpoint.type === 'Int' || setpoint.type === 'Float') {
      return (
        <NumericInput
          endTextAdornment={setpoint.unit}
          // FIXME: remove default end component adornment from NumericInput component in storybook
          // eslint-disable-next-line react/jsx-no-useless-fragment
          endComponentAdorment={<></>}
          label={labels.modeInfo.setpointRow.value}
          onChange={(event) => updateSetpoint(
            setpoint.id,
            type,
            'update',
            'value',
            event,
            setpoint.type,
          )}
          size="small"
          validationRegEx={setpoint.type === 'Float' ? 'floats' : 'integers'}
          value={setpoint.value}
        />

      );
    }
    if (setpoint.type === 'Bool') {
      return (
        <Select
          label={labels.modeInfo.setpointRow.value}
          menuItems={['true', 'false']}
          onChange={(event) => updateSetpoint(
            setpoint.id,
            type,
            'update',
            'value',
            event,
            setpoint.type,
          )}
          value={setpoint.value}
        />
      );
    }
    return (
      <TextField
        label={labels.modeInfo.setpointRow.value}
        onChange={(event) => updateSetpoint(
          setpoint.id,
          type,
          'update',
          'value',
          event,
          setpoint.type,
        )}
        size="small"
        value={setpoint.value}
      />
    );
  };

  useEffect(() => {
    handleValueComponent();
  // TODO: Find way around this
  // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [setpoint.type]);

  return (
    <>
      <TableRow sx={{ '& > *': { borderBottom: 'none' } }}>
        <TableCell sx={{ bottomBorder: 'none', size: 'small' }}>
          <Typography>{setpoint.name}</Typography>
        </TableCell>
        <TableCell align="right">
          <IconButton
            icon={open ? 'ExpandLess' : 'ExpandMore'}
            onClick={() => setOpen(!open)}
          />
        </TableCell>
      </TableRow>
      <TableRow>
        <TableCell colSpan={2} style={{ paddingBottom: 0, paddingTop: 0 }}>
          <Collapse in={open} timeout="auto" unmountOnExit>
            <Box sx={{ margin: 1 }}>
              <Table>
                <TableBody>
                  <TableRow>
                    <TableCell>
                      <Typography variant="body1">{labels.modeInfo.setpointRow.name}</Typography>
                    </TableCell>
                    <TableCell>
                      <TextField
                        label={labels.modeInfo.setpointRow.name}
                        onChange={(event) => updateSetpoint(
                          setpoint.id,
                          type,
                          'update',
                          'name',
                          event,
                        )}
                        size="small"
                        value={setpoint.name}
                      />
                    </TableCell>
                  </TableRow>
                  <TableRow>
                    <TableCell>
                      <Typography variant="body1">{labels.modeInfo.setpointRow.type}</Typography>
                    </TableCell>
                    <TableCell>
                      <Select
                        label={labels.modeInfo.setpointRow.type}
                        menuItems={SetpointTypes}
                        onChange={(event) => updateSetpoint(
                          setpoint.id,
                          type,
                          'update',
                          'type',
                          event,
                        )}
                        value={setpoint.type}
                      />
                    </TableCell>
                  </TableRow>
                  <TableRow>
                    <TableCell>
                      <Typography variant="body1">{labels.modeInfo.setpointRow.unit}</Typography>
                    </TableCell>
                    <TableCell>
                      <TextField
                        label={labels.modeInfo.setpointRow.unit}
                        onChange={(event) => updateSetpoint(
                          setpoint.id,
                          type,
                          'update',
                          'unit',
                          event,
                        )}
                        size="small"
                        value={setpoint.unit}
                      />
                    </TableCell>
                  </TableRow>
                  <TableRow>
                    <TableCell>
                      <Typography variant="body1">{labels.modeInfo.setpointRow.URI}</Typography>
                    </TableCell>
                    <TableCell>
                      <TextField
                        label={labels.modeInfo.setpointRow.URI}
                        onChange={(event) => updateSetpoint(
                          setpoint.id,
                          type,
                          'update',
                          'uri',
                          event,
                        )}
                        size="small"
                        value={setpoint.uri}
                      />
                    </TableCell>
                  </TableRow>
                  <TableRow>
                    <TableCell>
                      <Typography variant="body1">{labels.modeInfo.setpointRow.value}</Typography>
                    </TableCell>
                    <TableCell>
                      {handleValueComponent()}
                    </TableCell>
                  </TableRow>
                </TableBody>
                <TableFooter>
                  <TableCell align="left" sx={{ borderBottom: 'none' }}>
                    <MuiButton
                      color="error"
                      label={labels.modeInfo.setpointRow.delete}
                      onClick={() => updateSetpoint(setpoint.id, type, 'delete')}
                      size="small"
                      startIcon="Trash"
                      variant="text"
                    />
                  </TableCell>
                </TableFooter>
              </Table>
            </Box>
          </Collapse>
        </TableCell>
      </TableRow>
    </>
  );
};

export default SetpointRow;
