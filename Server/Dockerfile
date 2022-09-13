From python:3.8-alpine

COPY ./requirements.txt /app/requirements.txt

WORKDIR /app

RUN apk add build-base

RUN pip install -r requirements.txt

COPY . /app

ENTRYPOINT [ "python" ]

CMD ["run.py","test","test"]