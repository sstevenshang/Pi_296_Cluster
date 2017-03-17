#!/bin/bash

git add .
echo "enter commit message: "
read message
git commit -m "$message"
git push origin steven-branch

