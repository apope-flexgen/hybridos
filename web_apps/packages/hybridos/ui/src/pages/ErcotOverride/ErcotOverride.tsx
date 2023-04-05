// TODO - resolve these
/* eslint-disable max-lines, max-statements */
import {
  Box,
  IconButton,
  Select,
  TextField,
  Typography,
  DataTable,
  Switch,
  PageLoadingIndicator,
  CardContainer,
  CheckXConfirm,
} from '@flexgen/storybook';
import {
  useCallback, useEffect, useState,
} from 'react';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import QueryService from 'src/services/QueryService';
import {
  variableOverrideColumns,
  variableOverrideLabels,
  VariableOverrideRow,
  SITE_NAMES_URL,
  VARIABLE_NAMES_URL,
  VARIABLE_VALUES_URL,
  UPDATE_VARIABLE_OVERRIDE_URL,
} from './Constants';
import UnconfiguredContainer from './UnconfiguredContainer/UnconfiguredContainer';
import { PageProps } from 'src/pages/PageTypes';
import { Roles } from 'shared/types/api/Users/Users.types';

const ErcotOverride: React.FunctionComponent<PageProps> = ({ currentUser }: PageProps) => {
  const axiosInstance = useAxiosWebUIInstance();
  const { role } = currentUser;

  const [isLoading, setIsLoading] = useState<boolean>(false);
  const [siteNames, setSiteNames] = useState<string[]>([]);
  const [rowsData, setRowsData] = useState<VariableOverrideRow[]>([]);
  const [variableNames, setVariableNames] = useState<string[]>([]);
  const [variableValues, setVariableValues] = useState<{ [key: string]: any }>({});
  const [overrideEdits, setOverrideEdits] = useState<{ [key: string]: any }>({});
  const [enableEdit, setEnableEdit] = useState<{ [key:string]: boolean }>({});
  const [selectedSitename, setSelectedSiteName] = useState<string>('');

  const handleDataOnSocket = useCallback((newInformationFromSocket: MessageEvent) => {
    const data = JSON.parse(newInformationFromSocket.data);
    setVariableValues(data.data);
  }, []);

  const updateVariableOverride = (
    siteId: string,
    newValue: string | number | undefined | boolean,
    variableName: string,
  ) => {
    const newOverride = (typeof newValue === 'boolean') ? newValue : Number(newValue);
    axiosInstance.post(
      `${UPDATE_VARIABLE_OVERRIDE_URL}/${siteId}/${variableName}_manual`,
      { value: newOverride },
    );
  };

  const handleOverride = (
    enableEditValue: { [key:string]: boolean },
    variable: string,
    siteId: string,
  ) => {
    if (role !== Roles.Observer && enableEditValue[variable] && typeof variableValues[`${variable}_actual`] === 'number') {
      return (
        <Box sx={{ display: 'flex', flexDirection: 'row', alignItems: 'center' }}>
          <TextField
            label="Manual Override"
            placeholder={variableValues[`${variable}_manual`]}
            type="number"
            variant="standard"
            value={overrideEdits[variable]}
            onChange={(e) => {
              setOverrideEdits({ ...overrideEdits, [variable]: e.target.value });
            }}
          />
          <CheckXConfirm
            onCheck={() => {
              updateVariableOverride(siteId, overrideEdits[variable], variable);
              setEnableEdit({ ...enableEditValue, [variable]: false });
            }}
            onX={() => setEnableEdit({ ...enableEditValue, [variable]: false })}
            size="small"
          />
        </Box>
      );
    }
    else if (role !== Roles.Observer && enableEditValue[variable] && typeof variableValues[`${variable}_actual`] === 'boolean') {
      return (
        <Box sx={{ display: 'flex', flexDirection: 'row', alignItems: 'center' }}>
          <Select
            label="Manual Override"
            menuItems={["true", "false"]}
            value={overrideEdits[variable].toString()}
            onChange={(e) => {
              const newValue = e.target.value === 'true' ? true : false
              setOverrideEdits({ ...overrideEdits, [variable]: newValue });
            }}
            width="small"
          />
          <CheckXConfirm
            onCheck={() => {
              updateVariableOverride(siteId, overrideEdits[variable], variable);
              setEnableEdit({ ...enableEditValue, [variable]: false });
            }}
            onX={() => setEnableEdit({ ...enableEditValue, [variable]: false })}
            size="small"
          />
        </Box>
      );
    }
    return variableValues[`${variable}_manual`].toString();
  };

  const handleEnableOverride = (
    siteId: string,
    enabled: boolean | undefined,
    variableName: string,
  ) => {
    axiosInstance.post(
      `${UPDATE_VARIABLE_OVERRIDE_URL}/${siteId}/${variableName}_override`,
      { value: enabled },
    );
  };

  const generateRowData = (
    siteId: string,
    variableNamesFromAPI: string[],
    variableValuesFromAPI: { [key: string]: any },
  ) => {
    const tempRows: VariableOverrideRow[] = [];
    variableNamesFromAPI.map((variable) => {
      const newRow: VariableOverrideRow = {
        id: variable,
        variable_name: variable,
        ercot_standard: variableValues[`${variable}_actual`].toString(),
        current_value: variableValues[`${variable}_select`].toString(),
        enable: 
        <Switch
          disabled={role === Roles.Observer}
          color="primary"
          onChange={(value) => { handleEnableOverride(siteId, value, variable); }}
          value={variableValuesFromAPI[`${variable}_override`]}
        />,
        edit_override: 
        role !== Roles.Observer 
        ? <IconButton
            color="primary"
            icon="Edit"
            onClick={() => {
              setEnableEdit({ ...enableEdit, [variable]: true });
            }}
            size="small"
          />
        : <Box/>,
        manual_override: handleOverride(enableEdit, variable, siteId),
      };

      tempRows.push(newRow);
      return null;
    });
    setRowsData(tempRows);
  };

  const updateVariableData = async (siteId: string) => {
    try {
      setIsLoading(true);
      // set up socket connection
      QueryService.getVariableOverridePage(siteId, handleDataOnSocket);
      setSelectedSiteName(siteId);
      // get initial data to fill the table (socket will update data)
      const res = await axiosInstance.get(VARIABLE_NAMES_URL);
      const newVariableNames = res.data.sort();
      setVariableNames(newVariableNames);
      const variableValuesRes = await axiosInstance.get(`${VARIABLE_VALUES_URL}/${siteId}`);

      const newVariableValues = variableValuesRes.data;
      setVariableValues(newVariableValues);
      let tempEnableEdits = {};
      let tempOverrideEdits = {};
      newVariableNames.map((variable: string) => {
        tempEnableEdits = { ...tempEnableEdits, [variable]: false };
        tempOverrideEdits = {
          ...tempOverrideEdits,
          [variable]: newVariableValues[`${variable}_manual`],
        };
        return null;
      });
      setEnableEdit(tempEnableEdits);
      setOverrideEdits(tempOverrideEdits);
      generateRowData(siteId, newVariableNames, newVariableValues);
    } catch (e: any) {
    } finally {
      setIsLoading(false);
    }
  };

  const fetchData = useCallback(async () => {
    try {
      setIsLoading(true);
      const res = await axiosInstance.get(SITE_NAMES_URL);
      setSiteNames(res.data);
    } catch (e: any) {
    } finally {
      setIsLoading(false);
    }
  }, [axiosInstance]);

  useEffect(() => {
    fetchData();
  }, [fetchData]);

  useEffect(() => () => {
    QueryService.cleanupSocket();
  }, []);

  useEffect(
    () => {
      generateRowData(selectedSitename, variableNames, variableValues);
    }, /* eslint-disable react-hooks/exhaustive-deps */
    [enableEdit, variableValues, overrideEdits],
  );

  return (
    <Box sx={{
      display: 'flex',
      flexDirection: 'column',
      alignItems: 'center',
      gap: '12px',
      width: '100%',
      marginBottom: '20px',
    }}
    >
      <PageLoadingIndicator isLoading={isLoading} type="primary" />
      <CardContainer
        flexDirection="column"
        styleOverrides={{ width: '90%', padding: '12px', gap: '12px' }}
      >
        <Typography
          text={variableOverrideLabels.siteSelectorTitle}
          variant="headingS"
        />
        <Select
          label={variableOverrideLabels.siteSelectorLabel}
          menuItems={siteNames}
          onChange={(e) => { updateVariableData(e.target.value); }}
          value={selectedSitename}
        />
      </CardContainer>
      <CardContainer
        flexDirection="column"
        styleOverrides={{ width: '90%', gap: '12px' }}
      >
        <Box sx={{ paddingTop: '12px', paddingLeft: '12px', backgroundColor: 'transparent' }}>
          <Typography text={variableOverrideLabels.pageLabel} variant="headingS" />
        </Box>
        <Box sx={{ width: '100%', backgroundColor: 'transparent' }}>
          { selectedSitename === ''
            ? <UnconfiguredContainer />
            : (
              <DataTable
                columns={variableOverrideColumns}
                dense
                pagination
                rowsPerPage={[50, 20, 10, 5]}
                rowsData={rowsData}
              />
            )}
        </Box>
      </CardContainer>
      <Box sx={{ width: '100%', backgorundColor: 'transprent', minHeight: '12px' }} />
    </Box>
  );
};

export default ErcotOverride;
