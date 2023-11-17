import { Box, Typography } from '@flexgen/storybook';
import { useEffect } from 'react';
import { titleBoxSx, pageSx } from './SystemStatus.styles';
import QueryService from 'src/services/QueryService';

const SystemStatus: React.FunctionComponent = () => {
    // TODO: replace with actual handler mechanism for new data
    const tempHandleData = (newDataFromSocket: any) => {
        console.log(newDataFromSocket)
    }

    useEffect(() => {
        QueryService.getSystemStatus(tempHandleData);

        return () => {
            QueryService.cleanupSocket();
        };
    }, []);

    return (
        <Box sx={pageSx}>
            <Box sx={titleBoxSx}>
                <Typography text='System Status' variant='headingL' />
            </Box>
        </Box>
    );
};

export default SystemStatus;
