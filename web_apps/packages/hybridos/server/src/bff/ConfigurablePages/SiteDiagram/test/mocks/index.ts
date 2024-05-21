import { SiteDiagramConfigBody, SiteDiagramData, Status } from '../../siteDiagram.types';
import { DisplayGroupDTO, TreeConfig } from 'shared/types/dtos/configurablePages.dto';
import { ConfigurablePageDTO } from 'shared/types/dtos/configurablePages.dto';

export const mockAssetType = 'asset';
export const mockURI = '/asset';
export const mockField = 'field';
export const mockLabel = 'label';
export const mockIndex = 0;
export const mockUnit = 'unit';
export const mockScaler = '1';
export const defaultStaticValue = '-';
export const mockType = 'progress';
export const mockNumericValue = '1.00';
export const fullMockURI = `${mockURI}${mockURI}${mockURI}`;

export const createMockDTOResponse = (
  hasStatic: boolean,
  tree?: TreeConfig,
): ConfigurablePageDTO => {
  if (hasStatic)
    return {
      hasStatic,
      tree,
      displayGroups: {
        [`${fullMockURI}`]: {
          displayName: mockLabel,
          treeId: mockField,
          status: mockStatusData(hasStatic),
        },
      },
    };
  return {
    hasStatic,
    displayGroups: {
      [`${fullMockURI}`]: {
        displayName: mockLabel,
        treeId: mockField,
        status: mockStatusData(hasStatic),
      },
    },
  };
};

export const mockFimsMsg = {
  body: {
    [`${mockAssetType}`]: mockNumericValue,
  },
};

export const mockStatusData = (hasStatic): DisplayGroupDTO['status'] => {
  if (hasStatic)
    return {
      [`${mockAssetType}`]: {
        static: {
          label: mockLabel,
          unit: mockUnit,
          type: mockType,
        },
        state: {
          value: defaultStaticValue,
        },
      },
    };
  return {
    [`${mockAssetType}`]: {
      state: {
        value: mockNumericValue,
      },
    },
  };
};

const createMockAssetsObject = (
  assetType: string = '',
  label: string = mockLabel,
  unit: string = mockUnit,
  scalar: string = '1',
): SiteDiagramData => {
  return {
    [assetType]: {
      baseURI: mockURI,
      items: [
        {
          name: label,
          uri: mockURI,
          treeId: mockField,
          statuses: [
            {
              name: label,
              units: unit,
              type: mockType,
              scalar,
              uri: mockURI,
            },
          ],
        },
      ],
    },
  };
};

const createMockTreeObject = (assetType: string = ''): { root: TreeConfig } => {
  return {
    root: {
      [assetType]: {
        children: [],
      },
    },
  };
};

export const mockDBIConfig: SiteDiagramConfigBody = {
  baseURI: mockURI,
  assets: createMockAssetsObject('asset'),
  tree: createMockTreeObject('asset'),
};

export const mockStaticDataObject: ConfigurablePageDTO = createMockDTOResponse(
  true,
  createMockTreeObject('asset').root,
);
export const mockNonStaticDataObject: ConfigurablePageDTO = createMockDTOResponse(false);

export const mockConfigStatuses: Status[] = createMockAssetsObject('asset').asset.items[0].statuses;
