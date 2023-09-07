export const UriIsRootOfUri = (uri: string, root: string): boolean => {
    if (uri.endsWith('/')) {
        uri = uri.slice(0, -1);
    }
    if (root.endsWith('/')) {
        root = root.slice(0, -1);
    }
    if (uri === root) {
        return true;
    }
    if (uri.startsWith(root) && uri.charAt(root.length) === '/') {
        return true;
    }
    return false;
};