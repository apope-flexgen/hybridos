import React from 'react';
import { withStyles } from 'tss-react/mui';
import { userRole } from '../../AppAuth';

import {
    Paper,
    Card,
    CardHeader,
    CardContent,
    Typography,
    AppBar,
    Table,
    TableBody,
    TableHead,
    TableContainer,
    Grid,
    Button,
    Divider,
    IconButton,
    CircularProgress,
} from '@mui/material';

import MuiAlert from '@mui/material/Alert';

import { ReportProblemOutlined, ErrorOutline } from '@mui/icons-material';

import SocketGridButton from '../component/SocketGridButton';
import DataTableRow from '../component/DataTableRow';
import DataConfirmInput from '../component/DataConfirmInput';
import DataAlert from '../component/DataAlert';
import Loading from '../component/Loading';

import { getDataForURI } from '../../AppConfig';

import { STYLES_GRID_VIEW } from '../styles';

import { capitalizeFirst } from '../helpers';

function Alert(props) {
    return <MuiAlert elevation={6} variant="filled" {...props} />;
}

class AssetsPage extends React.Component {
    constructor(props) {
        super(props);

        this.state = {
            showAlarms: false,
            showFaults: false,
            alarms: [],
            faults: [],
            selectedItem: 0,
            selectedData: null,
            isLoading: true,
            md: 4,
            px: '64px',
            lockedOut: [],
        };
    }

    componentDidMount() {
        getDataForURI('/dbi/ui_config/show_documents')
            .then((response) => response.json())
            .then((response) => {
                if (response.body.includes('assets')) {
                    // Get dashboard configuration
                    getDataForURI('/dbi/ui_config/assets')
                        .then((response) => response.json())
                        .then((response) => {
                            if (
                                response.body.data &&
                                response.body.data.length !== 0
                            ) {
                                const { data } = response.body;
                                let pageData = null;
                                for (const page of data) {
                                    if (
                                        page.info.assetKey ===
                                        this.props.pageKey
                                    )
                                        pageData = page;
                                }
                                if (pageData !== null) {
                                    let md;
                                    let px;
                                    if (pageData.info.numberOfItems < 9) {
                                        md = 6;
                                        px = '92px';
                                    } else if (
                                        pageData.info.numberOfItems > 16
                                    ) {
                                        md = 3;
                                        px = '48px';
                                    } else {
                                        md = 4;
                                        px = '64px';
                                    }
                                    this.setState({
                                        info: pageData.info,
                                        statuses: pageData.statuses,
                                        summary: pageData.summary,
                                        controls: pageData.controls,
                                        summaryControls:
                                            pageData.summaryControls,
                                        allControls: pageData.allControls,
                                        alerts: {
                                            alarmFields:
                                                pageData.info.alarmFields,
                                            faultFields:
                                                pageData.info.faultFields,
                                        },
                                        // alerts: pageData.alarms,
                                        isLoading: false,
                                        md,
                                        px,
                                    });
                                } else {
                                    console.log(
                                        `${this.props.page} IS NOT CONFIGURED`
                                    );
                                    this.setState({ isLoading: false });
                                }
                            } else this.setState({ isLoading: false });
                        });
                } else {
                    this.setState({ isLoading: false });
                }
            })
            .catch((error) => {
                throw new Error(`ERROR in Assets/${this.props.page}: ${error}`);
            });
    }

    toggleAlarms = () => {
        this.setState({ showAlarms: !this.state.showAlarms });
    };

    toggleFaults = () => {
        this.setState({ showFaults: !this.state.showFaults });
    };

    selectItem = (item) => {
        this.setState({
            selectedItem: item === this.state.selectedItem ? 0 : item,
            selectedData: null,
        });
    };

    setLockedOut = (lockedOut, item) => {
        let updated = this.state.lockedOut;
        updated[item] = lockedOut;
        this.setState({
            lockedOut: updated,
        });
    };

    updateFromButton = (data, alarms, faults) => {
        this.setState({
            selectedData: data,
            alarms,
            faults,
        });
    };

    render() {
        const { classes } = this.props;
        const {
            showAlarms,
            showFaults,
            selectedItem,
            info,
            statuses,
            summary,
            controls,
            summaryControls,
            allControls,
            alerts,
            md,
            px,
            selectedData,
            alarms,
            faults,
            isLoading,
        } = this.state;
        const { toggleAlarms, toggleFaults, selectItem, updateFromButton } =
            this;

        const baseURI =
            info &&
            `${info.baseURI}${
                selectedItem === 'summary' ? '/' : info.extension
            }${selectedItem}`;

        return (
            <React.Fragment>
                {isLoading && (
                    <CircularProgress
                        size={100}
                        style={{ marginLeft: 550, marginTop: 250 }}
                    />
                )}
                {!isLoading && (
                    <div style={{ display: 'flex' }}>
                        <Paper
                            square
                            style={{
                                display: 'flex',
                                height: '84vh',
                                width: '75vw',
                                marginRight: '20px',
                            }}
                        >
                            <TableContainer
                                style={{
                                    maxHeight: '100%',
                                    paddingLeft: '20px',
                                }}
                                className={classes.scroll}
                            >
                                <Table>
                                    <TableHead
                                        style={{
                                            width: '100%',
                                            display: 'flex',
                                            alignItems: 'center',
                                            height: '80px',
                                        }}
                                    >
                                        <Typography
                                            variant="h6"
                                            gutterBottom
                                            component="div"
                                        >
                                            {info &&
                                                `${info.name} ${
                                                    selectedItem === 0
                                                        ? 'Page'
                                                        : selectedItem ===
                                                          'summary'
                                                        ? capitalizeFirst(
                                                              selectedItem
                                                          )
                                                        : info.itemName +
                                                          ' ' +
                                                          selectedItem
                                                }`}
                                            {!info &&
                                                `${this.props.page} page is not configured`}
                                        </Typography>
                                    </TableHead>
                                    <TableBody>
                                        {selectedItem !== 'summary' &&
                                            statuses &&
                                            statuses.map((status, index) => (
                                                <DataTableRow
                                                    status={status}
                                                    data={
                                                        selectedData &&
                                                        selectedData[
                                                            status.uri.substring(
                                                                1
                                                            )
                                                        ]
                                                    }
                                                />
                                            ))}
                                        {selectedItem === 'summary' &&
                                            summary &&
                                            summary.map((status, index) => (
                                                <DataTableRow
                                                    status={status}
                                                    data={
                                                        selectedData &&
                                                        selectedData[
                                                            status.uri.substring(
                                                                1
                                                            )
                                                        ]
                                                    }
                                                />
                                            ))}
                                    </TableBody>
                                </Table>
                            </TableContainer>
                            <Card
                                style={{
                                    display: 'flex',
                                    flexDirection: 'column',
                                    height: '94%',
                                    width: '100%',
                                    margin: '20px',
                                }}
                            >
                                <CardHeader
                                    title="Controls"
                                    className={classes.card_header}
                                />
                                <CardContent
                                    className={classes.scroll}
                                    style={{
                                        display: 'flex',
                                        flexDirection: 'column',
                                        flexGrow: 1,
                                        overflowY: 'auto',
                                    }}
                                >
                                    {selectedItem !== 'summary' &&
                                        selectedItem !== 'allcontrols' &&
                                        controls &&
                                        controls.map((control, index) => (
                                            <DataConfirmInput
                                                inputType={control.inputType}
                                                name={control.name}
                                                URI={control.uri}
                                                sourceURI={info.sourceURI}
                                                baseURI={`${info.baseURI}${
                                                    info.extension[
                                                        info.extension.length -
                                                            1
                                                    ] === '0' &&
                                                    selectedItem >= 10
                                                        ? info.extension.slice(
                                                              0,
                                                              -1
                                                          )
                                                        : info.extension
                                                }${selectedItem}`}
                                                disabled={
                                                    selectedItem === 0 ||
                                                    userRole === 'observer'
                                                }
                                                lockedOut={this.state.lockedOut}
                                                data={
                                                    selectedData &&
                                                    selectedData[
                                                        control.uri.substring(1)
                                                    ]
                                                }
                                                units={control.units}
                                                scalar={control.scalar}
                                                numberOfItems={
                                                    info.numberOfItems
                                                }
                                                selectedItem={selectedItem}
                                                setLockedOut={this.setLockedOut}
                                            />
                                        ))}
                                    {selectedItem === 'summary' &&
                                        summaryControls &&
                                        summaryControls.map(
                                            (control, index) => (
                                                <DataConfirmInput
                                                    inputType={
                                                        control.inputType
                                                    }
                                                    name={control.name}
                                                    URI={control.uri}
                                                    sourceURI={info.sourceURI}
                                                    baseURI={`${info.baseURI}/summary`}
                                                    disabled={
                                                        selectedItem === 0
                                                    }
                                                    data={
                                                        selectedData &&
                                                        selectedData[
                                                            control.uri.substring(
                                                                1
                                                            )
                                                        ]
                                                    }
                                                    units={control.units}
                                                    scalar={control.scalar}
                                                    numberOfItems={
                                                        info.numberOfItems
                                                    }
                                                    selectedItem={selectedItem}
                                                />
                                            )
                                        )}
                                    {selectedItem === 'allcontrols' &&
                                        allControls &&
                                        allControls.map((control, index) => (
                                            <DataConfirmInput
                                                inputType={control.inputType}
                                                name={control.name}
                                                URI={control.uri}
                                                sourceURI={info.sourceURI}
                                                baseURI={`${info.baseURI}${info.extension}`}
                                                disabled={selectedItem === 0}
                                                data={
                                                    selectedData &&
                                                    selectedData[
                                                        control.uri.substring(1)
                                                    ]
                                                }
                                                units={control.units}
                                                scalar={control.scalar}
                                                numberOfItems={
                                                    info.numberOfItems
                                                }
                                                selectedItem={selectedItem}
                                            />
                                        ))}
                                </CardContent>
                            </Card>
                        </Paper>
                        <Paper
                            square
                            style={{
                                display: 'flex',
                                flexDirection: 'column',
                                height: '84vh',
                                width: '22vw',
                            }}
                        >
                            <AppBar
                                position="static"
                                style={{
                                    height: '64px',
                                    display: 'flex',
                                    justifyContent: 'center',
                                    alignItems: 'center',
                                    boxShadow: 'none',
                                }}
                            >
                                <Typography variant="h5">
                                    {info && `${info.name} Page`}
                                </Typography>
                            </AppBar>
                            <div
                                className={classes.scroll}
                                style={{
                                    display: 'flex',
                                    flexDirection: 'column',
                                    flexGrow: 1,
                                    overflowY: 'auto',
                                }}
                            >
                                <Grid
                                    container
                                    spacing={2}
                                    style={{
                                        paddingTop: '20px',
                                        paddingLeft: '20px',
                                        width: '100%',
                                    }}
                                >
                                    <Grid
                                        item
                                        md={12}
                                        style={{
                                            display: 'flex',
                                            alignItems: 'center',
                                            justifyContent: 'center',
                                        }}
                                    >
                                        {info && info.hasSummary && (
                                            <SocketGridButton
                                                item="summary"
                                                selectedItem={selectedItem}
                                                selectItem={selectItem}
                                                baseURI={`${info.baseURI}/summary`}
                                                sourceURI={info.sourceURI}
                                                alarmFields={alerts.alarmFields}
                                                faultFields={alerts.faultFields}
                                                updateFromButton={
                                                    updateFromButton
                                                }
                                            />
                                        )}
                                    </Grid>
                                    <Grid
                                        item
                                        md={12}
                                        style={{
                                            display: 'flex',
                                            alignItems: 'center',
                                            justifyContent: 'center',
                                        }}
                                    >
                                        {info && info.hasAllControls && (
                                            <Button
                                                variant={
                                                    selectedItem ===
                                                    'allcontrols'
                                                        ? 'outlined'
                                                        : 'contained'
                                                }
                                                onClick={() =>
                                                    selectItem('allcontrols')
                                                }
                                            >
                                                All Controls
                                            </Button>
                                        )}
                                    </Grid>
                                    {info &&
                                        Array.from(
                                            { length: info.numberOfItems },
                                            (_, i) => i + 1
                                        ).map((item) => (
                                            <Grid
                                                item
                                                md={md}
                                                style={{
                                                    display: 'flex',
                                                    alignItems: 'center',
                                                    justifyContent: 'center',
                                                }}
                                            >
                                                <SocketGridButton
                                                    px={px}
                                                    item={item}
                                                    selectedItem={selectedItem}
                                                    selectItem={selectItem}
                                                    baseURI={`${info.baseURI}${
                                                        info.extension[
                                                            info.extension
                                                                .length - 1
                                                        ] === '0' && item >= 10
                                                            ? info.extension.slice(
                                                                  0,
                                                                  -1
                                                              )
                                                            : info.extension
                                                    }${item}`}
                                                    sourceURI={info.sourceURI}
                                                    alarmFields={
                                                        alerts.alarmFields
                                                    }
                                                    faultFields={
                                                        alerts.faultFields
                                                    }
                                                    updateFromButton={
                                                        updateFromButton
                                                    }
                                                />
                                            </Grid>
                                        ))}
                                </Grid>
                            </div>
                            <Divider light />
                            <div
                                style={{
                                    display: 'flex',
                                    height: '80px',
                                    width: '100%',
                                    alignItems: 'center',
                                    justifyContent: 'center',
                                }}
                            >
                                <IconButton
                                    className={`${
                                        alarms.length !== 0 &&
                                        classes.iconAlarmActive
                                    }`}
                                    onClick={toggleAlarms}
                                    size="large"
                                >
                                    <ReportProblemOutlined fontSize="large" />
                                </IconButton>
                                <IconButton
                                    className={`${
                                        faults.length !== 0 &&
                                        classes.iconFaultActive
                                    }`}
                                    onClick={toggleFaults}
                                    size="large"
                                >
                                    <ErrorOutline fontSize="large" />
                                </IconButton>
                            </div>
                        </Paper>
                        {alerts && (
                            <DataAlert
                                open={showAlarms}
                                onClose={toggleAlarms}
                                level="alarms"
                                baseURI={baseURI}
                                sourceURI={info.sourceURI}
                                alerts={alarms}
                            />
                        )}
                        {alerts && (
                            <DataAlert
                                open={showFaults}
                                onClose={toggleFaults}
                                level="faults"
                                baseURI={baseURI}
                                sourceURI={info.sourceURI}
                                alerts={faults}
                            />
                        )}
                    </div>
                )}
            </React.Fragment>
        );
    }
}

export default withStyles(AssetsPage, STYLES_GRID_VIEW);
