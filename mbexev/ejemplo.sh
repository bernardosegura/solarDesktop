#!/bin/bash
mbexev $(xinput --list | grep -i 'optical mouse' | grep -i -m 1 -v 'consumer\|keyboard' | grep -o 'id=[0-9]\+' | grep -o '[0-9]\+')
# por motivos de que el id puede cambiar, se puede obtener como se indica, esto para un optical mouse
#se puede ir probando hasta obtener el id correcto y de esta manera reemplazar en el script para su ejecución de manera correcta
#si se ingresa al archivo de configuración startUp.json este puede ser ejecutado al iniciar el sistema. 