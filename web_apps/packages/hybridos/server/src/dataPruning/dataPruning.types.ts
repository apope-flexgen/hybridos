export type DataPruningMap = {
  [socketID: string]: {
    [className: string]: {
      [handlerName: string]: DataPruningStoredData;
    };
  };
};

export type DataPruningStoredData = {
  [key: string]: any;
};

export type SocketIdentifier = {
  socketID: string;
  className: string;
  handlerName: string;
};
