import { ConfigHistory as ConfigHistoryComponent } from '@flexgen/storybook'
import { PageProps } from '../pageTypes'
import { useGetConfigDataQuery } from '../../api/configManager'
import useGetComponentData from '../../components/customHooks/useGetComponentData'
import formatData from './helpers/formatData'
import initialData from './helpers/initialData'
import { StyledContainer, StyledConfigMContainer } from '../../styles/configHistory'

const ConfigHistoryView = ({ pageName }: PageProps) => {
    const { componentData, processingData, error, isLoading } = useGetComponentData({
        useGetCustomQuery: useGetConfigDataQuery,
        initialData,
        formatData,
    })
    let { configData, path } = componentData

    return (
        <StyledContainer>
            <h2>{pageName}</h2>
            {error ? (
                // TODO: create modal error in SB and use it here.
                <>Oh no, there was an error</>
            ) : isLoading || processingData ? (
                // TODO: Fix PageLoadingIndicator component in SB to use it here.
                <>Loading...</>
            ) : (
                componentData && (
                    <StyledConfigMContainer>
                        <ConfigHistoryComponent configData={configData} path={path} />
                    </StyledConfigMContainer>
                )
            )}
        </StyledContainer>
    )
}

export default ConfigHistoryView
