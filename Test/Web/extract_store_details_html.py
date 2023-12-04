import urllib.request
from html.parser import HTMLParser

class DescriptionHTMLParser(HTMLParser):
    def __init__(self):
        super().__init__()
        self.recording = 0
        self.html_content = []

    def handle_starttag(self, tag, attrs):
        if tag == 'li':
            # Check if this <li> tag is the one we want to start recording
            if any(name == 'class' and 'description' in value for name, value in attrs):
                self.recording += 1
                if self.recording == 1:
                    self.html_content.append(self.get_starttag_text())
            elif self.recording:
                # If already recording, increment for nested <li>
                self.recording += 1

        if self.recording:
            # Append the start tag text if recording
            self.html_content.append(self.get_starttag_text())

    def handle_endtag(self, tag):
        if tag == 'li' and self.recording:
            if self.recording > 1:
                # Decrement counter when exiting nested <li>
                self.recording -= 1
            elif self.recording == 1:
                # Stop recording after closing the outermost <li class="description">
                self.recording -= 1
                self.html_content.append(f'</{tag}>')
                return

        if self.recording:
            # Append the closing tag if recording
            self.html_content.append(f'</{tag}>')

    def handle_data(self, data):
        if self.recording:
            self.html_content.append(data)

def extract_description_html(url):
    parser = DescriptionHTMLParser()
    req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0'})
    with urllib.request.urlopen(req) as response:
        html = response.read().decode('utf-8')
        parser.feed(html)
    return ''.join(parser.html_content)

url = "https://www.daz3d.com/amelia-9-hd"  # Example DAZ store URL
description_html = extract_description_html(url)

print(description_html)
