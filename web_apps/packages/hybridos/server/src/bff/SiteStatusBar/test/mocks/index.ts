import {
  SiteStatusDataField,
  SiteStatusResponse,
} from 'src/bff/SiteStatusBar/sitestatus.interface';

export const mockSiteConfiguration = { units: {} };

export const mockURI = 'uri';
export const mockField = 'field';
export const mockLabel = 'label';
export const mockIndex = 0;
export const mockUnit = 'unit';
export const mockScaler = 1;

const createMockDataSource = (
  dataType: string = '',
  field: string = mockField,
  index: number = 0,
): SiteStatusDataField => {
  return {
    uri: mockURI,
    field,
    label: mockLabel,
    dataType,
    index,
  };
};
export const mockDataSource = createMockDataSource();
export const mockDataSourceNumber = createMockDataSource('number', 'field', 1);
export const mockDataSourceBoolean = createMockDataSource('boolean', 'field', 2);
export const mockDataSourceString = createMockDataSource('string', 'field', 0);
export const mockDataSourceInvalidField = createMockDataSource('', 'invalid');

export const mockValueString = 'valueString';
export const mockValueNumber = 1.23;
const mockValueNumberString = mockValueNumber.toString();
export const mockValueBoolean = false;
const mockValueBooleanString = mockValueBoolean.toString();

const createNakedValue = (value: any) => value;
export const mockNakedValueString = createNakedValue(mockValueString);
export const mockNakedValueNumber = createNakedValue(mockValueNumber);
export const mockNakedValueBoolean = createNakedValue(mockValueBoolean);

const createClothedValue = (value: any, unit?: string, scaler?: number | string) => {
  return {
    value,
    unit,
    scaler,
  };
};
export const mockClothedValueString = createClothedValue(mockValueString);
export const mockClothedValueNumber = createClothedValue(mockValueNumber, mockUnit, mockScaler);
export const mockClothedValueBoolean = createClothedValue(mockValueBoolean);

const createMockResponse = (dataObject: any): SiteStatusResponse => {
  return { data: dataObject };
};

const createMockDataSourceResponse = (value: string, index = 0, unit = ''): SiteStatusResponse => {
  return createMockResponse({
    dataPoints: {
      [`${mockURI}/${mockField}`]: {
        label: mockLabel,
        index,
        unit,
        value,
      },
    },
  });
};
export const mockStringResponse = createMockDataSourceResponse(mockValueString, 0);
export const mockNumberResponse = createMockDataSourceResponse(mockValueNumberString, 1);
export const mockClothedNumberResponse = createMockDataSourceResponse(
  mockValueNumberString,
  1,
  mockUnit,
);
export const mockBooleanResponse = createMockDataSourceResponse(mockValueBooleanString, 2);

const createFieldData = (field: string = mockField, data: any = {}) => {
  return {
    body: {
      [field]: data,
    },
  };
};
export const mockFieldData = createFieldData(mockField);

const createFimsMsg = (field: string = mockField, data: any = {}) => createFieldData(field, data);
export const mockDataSourceStringFimsMsg = createFimsMsg(mockField, mockValueString);
export const mockDataSourceNumberFimsMsg = createFimsMsg(mockField, mockValueNumber);
export const mockDataSourceBooleanFimsMsg = createFimsMsg(mockField, mockValueBoolean);

const mockActiveFaults = 'activeFaults';
const mockActiveAlarms = 'activeAlarms';
const mockSiteState = 'siteState';

export const mockSiteStateFimsMsg = {
  body: {
    active_faults: mockActiveFaults,
    active_alarms: mockActiveAlarms,
    site_state: mockSiteState,
  },
};

export const mockSiteStateResponse = createMockResponse({
  activeFaults: mockActiveFaults,
  activeAlarms: mockActiveAlarms,
  siteState: mockSiteState,
  siteStatusLabel: '',
});

export const mockConfig = {
  dataSources: [mockDataSourceString, mockDataSourceNumber, mockDataSourceBoolean],
};
export const mockDBIConfig = {
  dataSources: [
    createMockDataSource('string', 'field', -1),
    createMockDataSource('number', 'field', -1),
    createMockDataSource('boolean', 'field', -1),
  ],
};
