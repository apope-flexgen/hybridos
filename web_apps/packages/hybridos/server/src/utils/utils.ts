// we probably won't have any units smaller than pico or
// larger than tera, but this can easily be expanded as needed
const SIPrefixMap = {
    0.000000000001: 'p',
    0.000000001: 'n',
    0.000001: 'μ',
    0.001: 'm',
    0.01: 'c',
    0.1: 'd',
    1: '',
    10: 'da',
    100: 'h',
    1000: 'k',
    1000000: 'M',
    1000000000: 'G',
    1000000000000: 'T',
}

const SIPrefixMapReverse = {
    p: 0.000000000001,
    n: 0.000000001,
    μ: 0.000001,
    m: 0.001,
    c: 0.01,
    d: 0.1,
    '': 1,
    da: 10,
    h: 100,
    k: 1000,
    M: 1000000,
    G: 1000000000,
    T: 1000000000000,
}

const defaultUnits = {
    W: 'M',
    Wh: 'M',
    VAR: 'M',
    VARh: 'M',
    VA: 'M',
    VAh: 'M',
    'W/s': 'M',
}

// FIXME: we should not be getting siteconfig here, but I am not sure where to get it from
let siteConfiguration
;(async () => {
    siteConfiguration = await import(`${WEB_UI_JSON_CONFIG_PATH}`)
})()

export const getSIPrefixFromNumber = (prefixNumber: number): string => {
    return SIPrefixMap[prefixNumber] || ''
}

export const getScalerFromSIPrefix = (prefixString: string): number => {
    return SIPrefixMapReverse[prefixString] || 1
}

// this translates to display units, but DOES NOT include the unit label.
// the label given in the data is the target display unit
export const computeNakedValue = (value: number, scalar: number): string => {
    return (value / scalar).toFixed(2)
}

// this translates to display units, and includes the target display unit label SEPARATELY
export const computeClothedValue = (
    value: number,
    scalar: number,
    baseUnit: string
): { value: string; targetUnit: string } => {
    let baseValue = value * scalar

    if (baseUnit[0] in SIPrefixMapReverse) {
        baseValue = baseValue * SIPrefixMapReverse[baseUnit[0]]
        baseUnit = baseUnit.slice(1)
    }

    const unitsMap =
        siteConfiguration && 'units' in siteConfiguration ? siteConfiguration.units : defaultUnits
    const siPrefix = baseUnit in unitsMap ? unitsMap[baseUnit] : ''
    const displayValue =
        siPrefix in SIPrefixMapReverse ? baseValue / SIPrefixMapReverse[siPrefix] : baseValue
    return { value: displayValue.toFixed(2), targetUnit: siPrefix + baseUnit }
}

export const UriIsRootOfUri = (uri: string, root: string): boolean => {
    if (uri.endsWith('/')) {
        uri = uri.slice(0, -1)
    }
    if (root.endsWith('/')) {
        root = root.slice(0, -1)
    }
    if (uri === root) {
        return true
    }
    if (uri.startsWith(root) && uri.charAt(root.length) === '/') {
        return true
    }
    return false
}

export const TestUriIsRootOfUri = (): void => {
    const testCases = [
        { uri: '/a', root: '/a', expected: true },
        { uri: '/a', root: '/a/', expected: true },
        { uri: '/a', root: '/a/b', expected: false },
        { uri: '/a/b', root: '/a', expected: true },
        { uri: '/a/b', root: '/a/', expected: true },
        { uri: '/a/b', root: '/a/b', expected: true },
        { uri: '/a/b', root: '/a/b/', expected: true },
        { uri: '/a/b', root: '/a/b/c', expected: false },
        { uri: '/a/b/c', root: '/a', expected: true },
        { uri: '/a/b/c', root: '/a/', expected: true },
        { uri: '/a/b/c', root: '/a/b', expected: true },
        { uri: '/a/b/c', root: '/a/b/', expected: true },
        { uri: '/a/b/c', root: '/a/b/c', expected: true },
        { uri: '/a/b/c', root: '/a/b/c/', expected: true },
        { uri: '/a/b/c', root: '/a/b/c/d', expected: false },
        { uri: '/a/b/c', root: '/', expected: true },
        { uri: '/a/b/c', root: '', expected: true },
        { uri: '/a/b/c', root: 'a', expected: false },
        { uri: '/a/b/c', root: 'a/', expected: false },
        { uri: '/a/b/c', root: 'a/b', expected: false },
    ]

    testCases.forEach((testCase) => {
        const result = UriIsRootOfUri(testCase.uri, testCase.root)
        if (result !== testCase.expected) {
            throw new Error(
                `UriIsRootOfUri(${testCase.uri}, ${testCase.root}) returned ${result} but expected ${testCase.expected}`
            )
        }
    })
}
