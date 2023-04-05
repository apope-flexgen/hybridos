# This script is run in the context of `npm run <script>` from the project root.

ERROR_COUNT=0
for file in ./migrate-mongo/configs/*;
do
    {
        echo "RUNNING UP: ${file##*/}" &&
        migrate-mongo up -f "./migrate-mongo/configs/${file##*/}" &&
        echo "COMPLETED UP: ${file##*/}"
    } || {
        echo "FAILED UP: ${file##*/}"
        ((ERROR_COUNT++))
    }
done
echo "migrate-mongo up finished with $ERROR_COUNT error(s)."
