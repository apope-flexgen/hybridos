import React from 'react';
import { STYLES_FEATURES } from '../../styles';
import { withStyles } from 'tss-react/mui';
import PropTypes from 'prop-types';
import LoadingHOC from '../LoadingHOC';
import {
    Grid,
    Card,
    CardHeader,
    CardContent,
    CardActions,
    CircularProgress,
    Button,
    Table,
    TableBody,
} from '@mui/material';

// import FMTableRow from './FMTableRow';
import SocketTableRow from '../SocketTableRow';

class FleetManagerSummaryCard extends React.Component {
    constructor(props) {
        super(props);

        this.props.setLoading(true)
    }

    componentDidMount() {
        this.props.setLoading(false)
    }

    render() {
        const { name, baseURI, statuses, isLoading } = this.props;
        const { classes } = this.props;
        return (
            <Grid item md={6}>
                <Card style={{ padding: 10, paddingTop: 5 }}>
                    <CardHeader
                        title={name}
                    />
                    <CardContent style={{ minHeight: 325 }}>
                        {!isLoading
                            && <Table>
                                <TableBody>
                                    {statuses && statuses.map((status, index) => (
                                        <SocketTableRow
                                            status={status}
                                            baseURI={baseURI}
                                        />
                                    ))}
                                </TableBody>
                            </Table>
                        }
                    </CardContent>
                    <CardActions>
                        {/* <Button
                            // component={Link}
                            // to={page_path}
                            color="primary"
                            variant="outlined"
                            fullWidth
                        >
                            Testing
                        </Button> */}
                    </CardActions>
                </Card>
            </Grid>
        );
    }
}
FleetManagerSummaryCard.propTypes = {
    classes: PropTypes.object,
  };
  export default withStyles(LoadingHOC(FleetManagerSummaryCard), STYLES_FEATURES);
