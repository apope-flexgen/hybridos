import {
  Box, IconButton, Menu, MenuItem, Tooltip, Typography,
} from '@flexgen/storybook';
import React, {
  FC, useContext, MouseEvent, useState,
} from 'react';
import { EventsRequestParams } from 'shared/types/dtos/events.dto';
import { NotifContextType, NotifContext } from 'src/contexts/NotifContext';
import useAxiosWebUIInstance from 'src/hooks/useAxios';
import { buildURI } from 'src/pages/Events/EventsHeader/EventsHeader-helpers';
import {
  downloadTxt, downloadPdf, downloadXlsxCsv, getTimeSuffix,
} from './EventsDownload-helpers';
import { Format } from './EventsDownloadTypes';

interface EventsDownloadProps {
  setIsLoading: React.Dispatch<React.SetStateAction<boolean>>;
  filters: EventsRequestParams;
}

const EventsDownload: FC<EventsDownloadProps> = ({
  setIsLoading,
  filters,
}: EventsDownloadProps) => {
  const [anchorEl, setAnchorEl] = useState<Element | undefined>(undefined);
  const open = Boolean(anchorEl);
  const handleClick = (event?: MouseEvent<HTMLDivElement>) => setAnchorEl(event?.currentTarget);
  const handleClose = () => setAnchorEl(undefined);

  const axiosInstance = useAxiosWebUIInstance();
  const notifCtx = useContext<NotifContextType | null>(NotifContext);

  function downloadEventsHandler(data: Event[], format: Format, timeSuffix: string) {
    switch (format) {
      case 'txt':
        downloadTxt(data, timeSuffix);
        break;
      case 'pdf':
        downloadPdf(data, timeSuffix);
        break;
      case 'xlsx':
        downloadXlsxCsv(data, 'xlsx', timeSuffix);
        break;
      case 'csv':
        downloadXlsxCsv(data, 'csv', timeSuffix);
        break;
      default:
        notifCtx?.notif(
          'error',
          'Events download handler failed. Please narrow filter criteria and/or try again.',
        );
    }
  }

  function downloadFilteredEvents(format: Format) {
    const newLimitFilter: EventsRequestParams = {
      ...filters,
      page: 0,
      limit: Number.MAX_SAFE_INTEGER,
    };
    setIsLoading(true);
    const URI = buildURI(newLimitFilter);
    axiosInstance
      .get(URI)
      .then((res) => {
        downloadEventsHandler(
          res.data.data,
          format,
          getTimeSuffix(newLimitFilter.startTime, newLimitFilter.endTime),
        );
      })
      .catch((err) => {
        if (err.response?.data?.statusCode === 408) {
          notifCtx?.notif(
            'error',
            'Events download request timed out. Please narrow filter criteria and/or try again.',
          );
        }
      })
      .finally(() => {
        setIsLoading(false);
        handleClose();
      });
  }

  return (
    <>
      <Tooltip title="Download Filtered Events" arrow={false} placement="right">
        <IconButton
          size="medium"
          color="primary"
          icon="Download"
          enableHover
          onClick={handleClick}
        />
      </Tooltip>
      <Box sx={{}}>
        <Menu anchorEl={anchorEl} onClose={handleClose} open={open}>
          <MenuItem onClick={() => downloadFilteredEvents('txt')} color="primary">
            <Typography text="TXT" variant="bodyM" color="primary" />
          </MenuItem>
          <MenuItem onClick={() => downloadFilteredEvents('csv')} color="primary">
            <Typography text="CSV" variant="bodyM" color="primary" />
          </MenuItem>
          <MenuItem onClick={() => downloadFilteredEvents('xlsx')} color="primary">
            <Typography text="XLSX" variant="bodyM" color="primary" />
          </MenuItem>
          <MenuItem onClick={() => downloadFilteredEvents('pdf')} color="primary">
            <Typography text="PDF" variant="bodyM" color="primary" />
          </MenuItem>
        </Menu>
      </Box>
    </>
  );
};

export default EventsDownload;
