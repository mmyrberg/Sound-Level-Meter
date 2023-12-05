# Make sure to include the discord_webhook library in a layer for the function
# The Lib should be placed in a path "python/lib/python3.11/site-packages/",
# ziped and then uploaded to AWS

import os
import json
import requests

# AWS Lambda function to process IoT Core messages and send Discord notifications.
def lambda_handler(event, context):
    try:
        # Print entire event to CloudWatch Logs for debugging
        print("Event Content:", json.dumps(event, indent=2))

        # Access payload
        payload = event.get('AvgDecibel20s', 0.0)

        # Print payload content for debugging
        print("Payload Content:", json.dumps(payload))

        # Place payload in avg_decibel variable
        avg_decibel = payload
  
        # Check if average decibel value is over 80
        if avg_decibel > 80:
            handle_high_decibel(avg_decibel)
    
    # Print errors for debugging
    except Exception as e:
        print(f"Error: {str(e)}") 
        return {"statusCode": 500, "body": f"Error processing IoT Core message: {str(e)}"}

def handle_high_decibel(avg_decibel):
    
    # Round avg_decibel to 2 decimal places
    avg_decibel = round(avg_decibel, 2)
    
    # Customize Discord message content
    discord_message = f"High noise level alert! Average decibel (20s): {avg_decibel}"

    # Get discord webhook URL
    discord_webhook_url = os.environ.get('DISCORD_WEBHOOK_URL')

    # Send message to Discord
    try:
        response = requests.post(discord_webhook_url, json={"content": discord_message})

        if response.status_code == 204:
            print("Notification sent successfully")
        else:
            print(f"Discord response: {response.status_code}, {response.text}")
            
    except Exception as e:
        print(f"Error: {str(e)}")
        return {"statusCode": 500, "body": f"Error sending Discord notification: {str(e)}"}