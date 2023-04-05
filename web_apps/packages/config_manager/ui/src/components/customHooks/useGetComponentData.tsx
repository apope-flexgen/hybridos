import { useState, useEffect } from 'react'
import { UseGetComponentDataProps } from '.'

/*
 ** Custom-Hook to format the data received from a RTK-Query auto-generated hook.
 */
const useGetComponentData = ({
    useGetCustomQuery,
    useGetCustomQueryWithParams,
    queryParam,
    initialData,
    formatData,
}: UseGetComponentDataProps) => {
    const { data, error, isLoading } =
        queryParam && useGetCustomQueryWithParams
            ? useGetCustomQueryWithParams(queryParam)
            : useGetCustomQuery?.()

    const [componentData, setComponentData] = useState(initialData)
    const [processingData, setProcessingData] = useState(isLoading)

    useEffect(() => {
        if (data && !error) {
            setProcessingData(true)
            setComponentData(formatData(initialData, data))
            setProcessingData(false)
        }
    }, [data])

    return { componentData, processingData, isLoading, error }
}

export default useGetComponentData
