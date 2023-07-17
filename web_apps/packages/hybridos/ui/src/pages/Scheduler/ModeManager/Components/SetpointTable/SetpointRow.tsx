/* eslint-disable */
// TODO: fix lint
import {
  NumericInput,
  MuiButton,
  Select,
  TextField,
  IconButton,
  Typography,
} from '@flexgen/storybook';
import {
  Box,
  Collapse,
  Table,
  TableBody,
  TableCell,
  TableFooter,
  TableRow,
  Tooltip,
} from '@mui/material';
import { validateURI } from 'shared/functions/uriValidation';
import React, { useEffect, useState } from 'react';
import { schedulerConfigLabels as labels, uriRules } from 'src/pages/Scheduler/ModeManager/Helpers';
import { DeleteSetpointLabel, Setpoints, SetpointTypes } from 'src/pages/Scheduler/ModeManager/Types';
import { setpointRowStyles as styles } from 'src/pages/Scheduler/ModeManager/Styles'
import { nameRegex } from 'src/pages/Scheduler/SchedulerConfiguration/Helpers';

interface SetpointRowProps {
  duplicateName: boolean;
  setpoint?: any;
  updateSetpoint?: any;
  type: Setpoints;
  disableModeManager: boolean;
}

const SetpointRow: React.FC<SetpointRowProps> = ({
  duplicateName,
  setpoint,
  updateSetpoint,
  type,
  disableModeManager,
}: SetpointRowProps) => {
  const [open, setOpen] = useState(false);
  const [incomplete, setIncomplete] = useState<boolean>(false);

  // TODO: unsure if this is actually required but it gets rejected by the backend if it is not included.
  // could look into adding a default value in the future.
  const handleValueComponent = () => {
    const label = `${labels.modeInfo.setpointRow.value} (${setpoint.type})*`;
    if (setpoint.type === 'Int' || setpoint.type === 'Float') {
      return (
        <NumericInput
          fullWidth
          disabled={disableModeManager}
          helperText={(isNaN(Number(setpoint.value)) || setpoint.value === '') ? labels.modeInfo.setpointRow.required : undefined}
          endTextAdornment={setpoint.unit}
          color={(isNaN(Number(setpoint.value)) || setpoint.value === '') ? 'error' : 'primary'}
          label={label}
          onChange={(event) =>
            updateSetpoint(setpoint.id, type, 'update', 'value', event.target.value, setpoint.type)
          }
          size='small'
          validationRegEx={setpoint.type === 'Float' ? 'floats' : 'integers'}
          value={setpoint.value}
        />
      );
    }
    if (setpoint.type === 'Bool') {
      return (
        <Select
          disabled={disableModeManager}
          error={(setpoint.value === undefined || setpoint.value === '')}
          helperText={(setpoint.value === undefined || setpoint.value === '') ? labels.modeInfo.setpointRow.required : undefined}
          label={label}
          menuItems={['true', 'false']}
          onChange={(event) =>
            updateSetpoint(setpoint.id, type, 'update', 'value', event.target.value, setpoint.type)
          }
          fullWidth
          value={setpoint.value}
        />
      );
    }
    return (
      <TextField
        fullWidth
        disabled={disableModeManager}
        helperText={(setpoint.value === undefined || setpoint.value === '' || nameRegex.test(setpoint.value)) ? labels.modeInfo.setpointRow.required : undefined}
        label={label}
        color={(setpoint.value === undefined || setpoint.value === '' || nameRegex.test(setpoint.value)) ? 'error' : 'primary'}
        onChange={(event) => updateSetpoint(
          setpoint.id,
          type,
          'update',
          'value',
          event.target.value,
          setpoint.type,
        )}
        size="small"
        value={setpoint.value}
      />
    );
  };

  useEffect(() => {
    const temp =
      !setpoint.name ||
      !setpoint.uri ||
      !setpoint.type ||
      setpoint.value === undefined ||
      setpoint.value === '';
    setIncomplete(temp);
  }, [setpoint]);

  const handleNameFieldHelper = () => {
    if (!setpoint.name) return labels.modeInfo.setpointRow.required;
    else if (duplicateName) return labels.modeInfo.setpointRow.duplicateName;
    return undefined;
  };

  const handleURIFieldHelper = () => {
    if (!setpoint.uri) return labels.modeInfo.setpointRow.required;
    else if (!validateURI(setpoint.uri)) return labels.modeInfo.tooltip.invalidURI;
    return undefined;
  };

  return (
    <>
      <TableRow sx={styles.header.row}>
        <TableCell sx={styles.header.cell}>
          <Typography variant='bodyLBold' text={setpoint.name} />
        </TableCell>
        <TableCell align='right'>
          <IconButton icon={open ? 'ExpandLess' : 'ExpandMore'} onClick={() => setOpen(!open)} />
        </TableCell>
      </TableRow>
      <TableRow>
        <TableCell colSpan={2} style={styles.innerCell}>
          <Collapse in={open} timeout='auto' unmountOnExit>
            <Box sx={styles.innerBox}>
              <Table>
                <TableBody>
                  <TableRow>
                    <TableCell>
                      <Typography variant='bodyL' text={labels.modeInfo.setpointRow.name} />
                    </TableCell>
                    <TableCell>
                      <TextField
                        fullWidth
                        disabled={disableModeManager}
                        color={!setpoint.name || duplicateName ? 'error' : 'primary'}
                        label={labels.modeInfo.setpointRow.name}
                        helperText={handleNameFieldHelper()}
                        onChange={(event) =>
                          updateSetpoint(setpoint.id, type, 'update', 'name', event.target.value)
                        }
                        size='small'
                        required
                        value={setpoint.name}
                      />
                    </TableCell>
                  </TableRow>
                  <TableRow>
                    <TableCell>
                      <Typography variant='bodyL' text={labels.modeInfo.setpointRow.unit} />
                    </TableCell>
                    <TableCell>
                      <TextField
                        fullWidth
                        disabled={disableModeManager}
                        label={labels.modeInfo.setpointRow.unit}
                        onChange={(event) =>
                          updateSetpoint(setpoint.id, type, 'update', 'unit', event.target.value)
                        }
                        size='small'
                        value={setpoint.unit}
                      />
                    </TableCell>
                  </TableRow>
                  <TableRow>
                    <TableCell>
                      <Box sx={styles.uriTypographyBox}>
                        <Typography variant='bodyL' text={labels.modeInfo.setpointRow.URI} />
                        <Tooltip arrow title={uriRules}>
                          <div>
                            <IconButton icon='Help' size='small' />
                          </div>
                        </Tooltip>
                      </Box>
                    </TableCell>
                    <TableCell>
                      <TextField
                        fullWidth
                        disabled={disableModeManager}
                        color={!setpoint.uri ? 'error' : 'primary'}
                        /** TODO: eventually this should be replaced by a URI component since this has no checking for valid URIs. */
                        label={labels.modeInfo.setpointRow.URI}
                        helperText={handleURIFieldHelper()}
                        onChange={(event) =>
                          updateSetpoint(setpoint.id, type, 'update', 'uri', event.target.value)
                        }
                        size='small'
                        required
                        validator={() => validateURI(setpoint.uri)}
                        value={setpoint.uri}
                      />
                    </TableCell>
                  </TableRow>
                  <TableRow>
                    <TableCell>
                      <Typography variant='bodyL' text={labels.modeInfo.setpointRow.type} />
                    </TableCell>
                    <TableCell>
                      <Select
                        fullWidth
                        disabled={disableModeManager}
                        error={!setpoint.type}
                        label={labels.modeInfo.setpointRow.type}
                        helperText={
                          !setpoint.type ? labels.modeInfo.setpointRow.required : undefined
                        }
                        menuItems={SetpointTypes}
                        onChange={(event) =>
                          updateSetpoint(setpoint.id, type, 'update', 'type', event.target.value)
                        }
                        required
                        value={setpoint.type}
                      />
                    </TableCell>
                  </TableRow>
                  <TableRow>
                    <TableCell>
                      <Typography variant='bodyL' text={labels.modeInfo.setpointRow.value} />
                    </TableCell>
                    <TableCell>{handleValueComponent()}</TableCell>
                  </TableRow>
                  {incomplete && (
                    <TableRow>
                      <TableCell />
                      <TableCell>
                        <Typography
                          variant='helperText'
                          color='error'
                          text={labels.modeInfo.setpointRow.fillOutFields}
                        />
                      </TableCell>
                    </TableRow>
                  )}
                </TableBody>
                <TableFooter>
                  <TableCell align='left' sx={styles.deleteButton}>
                    <MuiButton
                      disabled={disableModeManager}
                      color='error'
                      label={DeleteSetpointLabel(type)}
                      onClick={() => updateSetpoint(setpoint.id, type, 'delete')}
                      size='small'
                      startIcon='Trash'
                      variant='text'
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
