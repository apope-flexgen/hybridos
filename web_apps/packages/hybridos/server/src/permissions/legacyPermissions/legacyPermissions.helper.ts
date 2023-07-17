/**
 * logic brought directly from old web_server
 *
 * returning the truncated and the original uri
 * some configs are not using any_site, but rather listing every possible site
 * this solution supports both options
 */
const legacyTruncateURI = (uri: string): string[] => {
  if (uri.includes('/dbi/')) {
    return ['/dbi'];
  } else if (uri.includes('/sites/') || uri.includes('/components/')) {
    // replaces the site name in uri with any_site ( /sites/batcave/active_power => /sites/any_site/active_power )
    const uriBase = uri.includes('/sites/') ? '/sites' : '/components';

    return [
      `${uriBase}/any_site${
        uri.split('/').length >= 3 ? `/${uri.split('/').slice(3, uri.length).join('/')}` : ''
      }`,
    ];
  }

  return [];
};

const schedulerTransformations = (uri: string): string[] => {
  return [
    ...schedulerConfigurationTransformations(uri),
    ...schedulerModesTransformations(uri),
    ...schedulerEventsTransformations(uri),
  ];
};

const schedulerConfigurationTransformations = (uri: string): string[] => {
  if (!uri.startsWith('/scheduler/configuration/web_sockets/clients/')) return [];

  const splitURI = uri.split('/');
  splitURI[5] = '{clientId}';
  return [splitURI.join('/')];
};

const schedulerModesTransformations = (uri: string): string[] => {
  if (!uri.startsWith('/scheduler/modes')) return [];

  const URIs = applyFunctions(uri, [replaceModeId, replaceSetpointType, replaceSetpointId]);

  return URIs;
};

const replaceModeId = (uri: string[]): string[] => {
  return uri.map((uri) => {
    const splitURI = uri.split('/');
    if (splitURI.length >= 4) splitURI[3] = '{modeId}';

    return splitURI.join('/');
  });
};

const replaceSetpointType = (uri: string[]): string[] => {
  return uri.map((uri) => {
    const splitURI = uri.split('/');
    if ((splitURI.length >= 5 && splitURI[4] === 'constants') || splitURI[4] === 'variables')
      splitURI[4] = '{setpointType}';

    return splitURI.join('/');
  });
};

const replaceSetpointId = (uri: string[]): string[] => {
  return uri.map((uri) => {
    const splitURI = uri.split('/');
    if (splitURI.length >= 6) splitURI[5] = '{setpointId}';

    return splitURI.join('/');
  });
};

const schedulerEventsTransformations = (uri: string): string[] => {
  if (!uri.startsWith('/scheduler/events')) {
    return [];
  }

  const URIs = applyFunctions(uri, [
    replaceScheduleID,
    replaceEventID,
    replaceVariableID,
    replaceExceptionIndex,
  ]);

  return URIs;
};

const replaceScheduleID = (uri: string[]): string[] => {
  return uri.map((uri) => {
    const splitURI = uri.split('/');
    if (splitURI.length >= 4) splitURI[3] = '{scheduleId}';

    return splitURI.join('/');
  });
};

const replaceEventID = (uris: string[]): string[] => {
  return uris.map((uri) => {
    const splitURI = uri.split('/');
    if (splitURI.length >= 5) splitURI[4] = '{eventId}';

    return splitURI.join('/');
  });
};

const replaceVariableID = (uris: string[]): string[] => {
  return uris.map((uri) => {
    const splitURI = uri.split('/');
    if (splitURI.length >= 7 && splitURI[5] === 'variables') splitURI[6] = '{variableId}';

    return splitURI.join('/');
  });
};

const replaceExceptionIndex = (uris: string[]): string[] => {
  return uris.map((uri) => {
    const splitURI = uri.split('/');
    if (splitURI.length >= 8 && splitURI[5] === 'repeat' && splitURI[6] === 'exceptions')
      splitURI[7] = '{exceptionIndex}';

    return splitURI.join('/');
  });
};

export const transformURI = (uri: string): string[] => {
  return [uri, ...legacyTruncateURI(uri), ...schedulerTransformations(uri)];
};

const applyFunctions = (originalValue: string, functions: any[]): string[] => {
  const runningResult = [originalValue];
  functions.forEach((func) => {
    const result: string[] = func(runningResult);
    result.forEach((transformedURI) => {
      if (!runningResult.includes(transformedURI)) runningResult.push(transformedURI);
    });
  });
  return runningResult.filter((value) => value !== originalValue);
};
