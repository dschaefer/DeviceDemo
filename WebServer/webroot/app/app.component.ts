import { Component, OnInit } from '@angular/core';

@Component({
    selector: 'my-app',
    template: `
        <h1 [ngStyle]="{ 'color': color }">{{color}}</h1>
        <button type="button" (click)="onClick()">{{command}}</button>
        `
}) export class AppComponent implements OnInit {
    color: string;
    socket: WebSocket;
    command: string;

    ngOnInit() {
        this.color = "initing";
        this.command = "Flash";
        this.socket = new WebSocket("ws://localhost:8090");
        this.socket.onmessage = (message: MessageEvent) => {
            this.color = message.data;
        };
    }
    
    onClick() {
        this.socket.send(this.command);
        if (this.command === "Flash") {
            this.command = "Off";
        } else {
            this.command = "Flash";
        }
    }
}
