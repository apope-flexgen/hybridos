# This script is run in the context of `npm run <script>` from the project root.

ERROR_COUNT=0
for file in ./migrate-mongo/configs/*;
do
    {
        echo "RUNNING DOWN: ${file##*/}" &&
        migrate-mongo down -f "./migrate-mongo/configs/${file##*/}" &&
        echo "COMPLETED DOWN: ${file##*/}"
    } || {
        echo "FAILED DOWN: ${file##*/}"
        ((ERROR_COUNT++))
    }
done
echo "migrate-mongo down finished with $ERROR_COUNT error(s)."
