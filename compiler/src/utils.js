// ns: [String]
export function buildEnum(ns) {
    const obj = {};
    for (const n of ns) {
        obj[n] = n;
    }
    return Object.freeze(obj);
}
